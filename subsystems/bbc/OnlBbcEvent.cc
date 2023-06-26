#include "OnlBbcEvent.h"

//#include <BbcOut.h>

//#include <getClass.h>
#include <Event/Event.h>
//#include <recoConsts.h>

#include <TRandom.h>
#include <TString.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>

#include <cmath>
#include <iostream>
#include <iomanip>

using namespace std;

// light velocity [cm/ns]
//static const float C = GSL_CONST_CGS_SPEED_OF_LIGHT / 1e9;

OnlBbcEvent::OnlBbcEvent(void) :
  EventNumber(0),
  calib_done(0),
  p{nullptr,nullptr},
  _tres(0.05),
  ac(nullptr)
{
  //set default values

  int nch = 256;
  int nsamples = 31;
  for (int ich=0; ich<nch; ich++)
  {
    //cout << "Creating bbcsig " << ich << endl;
    bbcsig.push_back( OnlBbcSig(ich,nsamples) );
    
    // Do evt-by-evt pedestal using sample range below
    bbcsig[ich].SetEventPed0Range(0,1);
  }

  TString name, title;
  for (int iarm = 0; iarm < 2; iarm++)
  {
    //
    name = "hevt_bbct";
    name += iarm;
    title = "bbc times, arm ";
    title += iarm;
    hevt_bbct[iarm] = new TH1F(name, title, 2000, -50., 50.);
    hevt_bbct[iarm]->SetLineColor(4);
  }
  h2_tmax[0] = new TH2F("h2_ttmax","time tmax vs ch",NSAMPLES,-0.5,NSAMPLES-0.5,128,0,128);
  h2_tmax[0]->SetXTitle("sample");
  h2_tmax[0]->SetYTitle("ch");
  h2_tmax[1] = new TH2F("h2_qtmax","chg tmax vs ch",NSAMPLES,-0.5,NSAMPLES-0.5,128,0,128);
  h2_tmax[1]->SetXTitle("sample");
  h2_tmax[1]->SetYTitle("ch");

  for (int iboard=0; iboard<16; iboard++)
  {
    TRIG_SAMP[iboard] = -1;
  }

  gaussian = nullptr;

  Clear();

  /*
  if ( verbose )
  {
    ac = new TCanvas("ac","ac",550*1.5,425*1.5);
    ac->Divide(2,1);
  }
  */
}

///
OnlBbcEvent::~OnlBbcEvent()
{
  for (int iarm=0; iarm<2; iarm++)
  {
    delete hevt_bbct[iarm];
  }

  delete h2_tmax[0];
  delete h2_tmax[1];
  delete ac;
  delete gaussian;

}

///
void OnlBbcEvent::Clear()
{
  ///reset BBC raw data
  for ( int ipmt = 0; ipmt < BBC_N_PMT; ipmt++ )
  {
    iHit[ipmt] = 0;
    armHitPmt[ipmt] = 0;
    Charge[ipmt] = 0;
    HitTime0[ipmt] = 0;
    HitTime1[ipmt] = 0;
  }
 

  ///Reset BBC hits information
  for ( int iarm = 0; iarm < 2; iarm++ )
  {
    nHitPmt[iarm] = 0;
    ChargeSum[iarm] = 0;
    ArmHitTime[iarm] = 0;
    ArmHitTimeError[iarm] = 0;
    f_bbcn[iarm] = 0;
  }
  ///Reset end product to prepare next event
  ZVertex = 0.0;
  ZVertexError = 0.0;
  TimeZero = 0.0;
  TimeZeroError = 0.0;

    // Arm Data
  f_bbcn[0] = 0;
  f_bbcn[1] = 0;
  f_bbcq[0] = 0.;
  f_bbcq[1] = 0.;
  f_bbct[0] = -9999.;
  f_bbct[1] = -9999.;
  f_bbcte[0] = -9999.;
  f_bbcte[1] = -9999.;
  f_bbcz = NAN;
  f_bbct0 = NAN;
  hevt_bbct[0]->Reset();
  hevt_bbct[1]->Reset();

  // PMT data
  std::fill_n(f_pmtt0, 128, 1e12);
  std::fill_n(f_pmtt1, 128, 1e12);
  std::fill_n(f_pmtq, 128, 0.);
}

bool OnlBbcEvent::setRawData(Event *event)
{
  // Get the relevant packets from the Event object and transfer the
  // data to the subsystem-specific table.

  //int flag_err = 0;
  for (int ipkt=0; ipkt<2; ipkt++)
  {
    int pktid = 1001 + ipkt;    // packet id
    p[ipkt] = event->getPacket( pktid );
    //cout << "Found packet " << 2001+ipkt << "\t" << p[ipkt] << endl;

    if ( p[ipkt] )
    {
      for (int ich=0; ich<NCHPERPKT; ich++)
      {
        int feech = ipkt*NCHPERPKT + ich;
        //cout << feech << endl;
        for (int isamp=0; isamp<NSAMPLES; isamp++)
        {
          f_adc[feech][isamp] = p[ipkt]->iValue(isamp,ich);
          f_samp[feech][isamp] = isamp;

          if ( f_adc[feech][isamp] <= 100 )
          {
            //flag_err = 1;
            //cout << "BAD " << f_evt << "\t" << feech << "\t" << f_samp[feech][isamp]
            //    << "\t" << f_adc[feech][isamp] << endl;
          }
        }
      }
    }
    else
    {
      //flag_err = 1;
      //cout << "ERROR, evt " << f_evt << " Missing Packet " << pktid << endl;
    }
  }

  // Delete the packets
  for (int ipkt=0; ipkt<2; ipkt++)
  {
    if ( p[ipkt]!=0 ) delete p[ipkt];
  }

  EventNumber++;
  return 0;

}

///
int OnlBbcEvent::calculate()
{
  //cout << "In OnlBbcEvent::calculate()" << endl;
  Clear();
  if ( ! gaussian )
  {
    gaussian = new TF1("gaussian", "gaus", 0, 20);
    gaussian->FixParameter(2, _tres);  // set sigma to timing resolution
  }

  for (int ich=0; ich<256; ich++)
  {
    bbcsig[ich].SetXY(f_samp[ich],f_adc[ich]);

    // determine the trig_samp board by board
    int tq = (ich/8)%2;   // 0 = T-channel, 1 = Q-channel
    int pmtch = (ich/16)*8 + ich%8;

    double x, y;
    bbcsig[ich].LocMax(x,y);
    h2_tmax[tq]->Fill(x,pmtch);
  }

  std::vector<float> hit_times[2];  // times of the hits in each [arm]
  
  if ( h2_tmax[0]->GetEntries() == 128*100 )
  {
    TString name;
    TH1 *h_trigsamp[16]{};
    for (int iboard=0; iboard<16; iboard++)
    {
      name = "h_trigsamp"; name += iboard;
      h_trigsamp[iboard] = h2_tmax[0]->ProjectionX( name, iboard*8 + 1, (iboard+1)*8 );
      int maxbin = h_trigsamp[iboard]->GetMaximumBin();
      TRIG_SAMP[iboard] = h_trigsamp[iboard]->GetBinCenter( maxbin );
      //std::cout << "iboard " << iboard << "\t" << iboard*8+1 << "\t" << (iboard+1)*8 << "\t" << h_trigsamp[iboard]->GetEntries() << std::endl;
      cout << "TRIG_SAMP" << iboard << "\t" << TRIG_SAMP[iboard] << endl;
    }

    calib_done = 1;
  }

  if ( calib_done == 0 )
  {
    return 0;
  }

  //for (int ich=0; ich<NCH; ich++)
  for (int ich=0; ich<256; ich++)
  {
    int arm = ich/128;    // south or north
    int board = ich/16;    // south or north
    //int quad = ich/64;    // quadrant
    int pmtch = (ich/16)*8 + ich%8;
    int tq = (ich/8)%2;   // 0 = T-channel, 1 = Q-channel

    /*
    // correct for offset of max samp
    if ( board==15 || board==13 )
    {
      //if ( ich==255 ) cout << "trig_samp " << TRIG_SAMP << endl;
      TRIG_SAMP = 16.;
    }
    else
    {
      TRIG_SAMP = 17.;
    }
    */

    if ( tq == 1 ) // Use dCFD method to get time for now in charge channels
    {
      //Double_t threshold = 4.0*sig->GetPed0RMS();
      //
      bbcsig[ich].GetSplineAmpl();
      Double_t threshold = 0.5;
      //f_t0[ich] = bbcsig[ich].dCFD( threshold ) - (TRIG_SAMP - 2);
      f_pmtt1[pmtch] = bbcsig[ich].dCFD( threshold ) - (TRIG_SAMP[board] - 2);
      if ( board==10 || board==11 || board==14 )
      {
        //f_t0[ich] += 1;
        f_pmtt1[pmtch] += 1;
      }
      else if ( board==13 )
      {
        //f_t0[ich] -= 1;
        f_pmtt1[pmtch] -= 1;
      }
      //f_t0[ich] *= 17.7623;               // convert from sample to ns (1 sample = 1/56.299 MHz)
      f_pmtt1[pmtch] *= 17.7623;               // convert from sample to ns (1 sample = 1/56.299 MHz)
      f_ampl[ich] = bbcsig[ich].GetAmpl();
      f_pmtq[pmtch] = f_ampl[ich]/100.; // for now, calibration is 100

      if ( f_ampl[ich]<40 || (f_pmtt1[pmtch]<-25.||f_pmtt1[pmtch]>25.) )
      {
        //f_t0[ich] = -9999.;
        f_pmtt1[pmtch] = -9999.;
      }
      else
      {
        //if ( f_pmtt1[pmtch]<-50. && ich==255 ) cout << "hit_times " << ich << "\t" << f_pmtt1[pmtch] << endl;
        //if ( arm==1 ) cout << "hit_times " << ich << "\t" << setw(10) << f_pmtt1[pmtch] << "\t" << board << "\t" << TRIG_SAMP[board] << endl;
        hit_times[arm].push_back( f_pmtt1[pmtch] );
        hevt_bbct[arm]->Fill( f_pmtt1[pmtch] );
        f_bbcn[arm]++;

        ChargeSum[arm] += f_pmtq[pmtch]; // for now, calibration is 100

      }

      if ( EventNumber<3 && ich==255 && f_ampl[ich] )
      {
          cout << "dcfdcalc " << EventNumber << "\t" << ich << "\t" << f_pmtt1[pmtch] << "\t" << f_ampl[ich] << endl;
      }
    }
    else // Use MBD method to get time in time channels
    {
      //Double_t threshold = 4.0*bbcsig[ich].GetPed0RMS();
      //Double_t threshold = 0.5

      //const int TRIG_SAMP = 14;     // use this sample to get the time
      Double_t tdc = bbcsig[ich].MBD( TRIG_SAMP[board] );
      f_ampl[ich] = bbcsig[ich].GetSplineAmpl();
      //if ( ich==0 ) cout << "XXX " << tdc << endl;

      if ( tdc<100 )
      {
          f_pmtt0[pmtch] = -9999.;   // No Hit
      }
      else
      {
          //chiu Skip for now
          // Convert TDC to ns
          //f_pmtt0[pmtch] = tdc2time[ich]->Eval( tdc );
          f_pmtt0[pmtch] = 12.5 - tdc*0.00189;  // simple linear correction
          //hit_times[arm].push_back( f_pmtt0[pmtch] );
          //hevt_bbct[arm]->Fill( f_pmtt0[pmtch] );
          //f_bbcn[arm]++;
      }
    }
  }

  //cout << "nhits " << f_bbcn[0] << "\t" << f_bbcn[1] << endl;
  //cout << "bbcte " << f_bbcte[0] << "\t" << f_bbcte[1] << endl;

  for (int iarm = 0; iarm < 2; iarm++)
  {
    if ( hit_times[iarm].empty() )
    {
        //cout << "hit_times size == 0" << endl;
        continue;
    }

    //cout << "EARLIEST " << iarm << endl;
    //cout << "ERROR hit_times size == " << hit_times[iarm].size() << endl;

    //std::sort(hit_times[iarm].begin(), hit_times[iarm].end());
    float earliest = hit_times[iarm].at(0);
    //cout << "earliest" << iarm << "\t" << earliest << endl;

    gaussian->SetParameter(0, 5);
    gaussian->SetParameter(1, earliest);
    gaussian->SetRange(6, earliest + 5 * 0.05);
    // gaussian->SetParameter(1,hevt_bbct[iarm]->GetMean());
    // gaussian->SetRange(6,hevt_bbct[iarm]->GetMean()+0.125);

    if ( ac ) ac->cd(iarm+1);
    hevt_bbct[iarm]->Fit(gaussian, "BNQLR");
    //hevt_bbct[iarm]->Draw();

    // f_bbct[iarm] = f_bbct[iarm] / f_bbcn[iarm];
    f_bbct[iarm] = gaussian->GetParameter(1);
    f_bbcte[iarm] = earliest;

    //_bbcout->AddBbcNS(iarm, f_bbcn[iarm], f_bbcq[iarm], f_bbct[iarm]);
  }

  // Get Zvertex, T0
  if (f_bbcn[0] > 0 && f_bbcn[1] > 0)
  {
    /*
    // Now calculate zvtx, t0 from best times
    cout << "t0\t" << f_bbct[0] << "\t" << f_bbct[1] << endl;
    f_bbcz = (f_bbct[0] - f_bbct[1]) * C / 2.0;
    f_bbct0 = (f_bbct[0] + f_bbct[1]) / 2.0;
    */

    //cout << "t0\t" << f_bbct[0] << "\t" << f_bbct[1] << endl;
    //cout << "te\t" << f_bbcte[0] << "\t" << f_bbcte[1] << endl;
    f_bbcz = (f_bbcte[0] - f_bbcte[1]) * TMath::C() * 1e-7 / 2.0; // in cm
    f_bbct0 = (f_bbcte[0] + f_bbcte[1]) / 2.0;

    //cout << "bbcz " << f_bbcz << endl;
    //_bbcout->set_Vertex(f_bbcz, 0.6);
    //_bbcout->set_TimeZero(f_bbct0, 0.05);
  }


  return 0;
}

