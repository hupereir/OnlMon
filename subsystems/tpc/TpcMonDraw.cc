#include "TpcMonDraw.h"

#include <onlmon/OnlMonClient.h>
#include <onlmon/OnlMonDB.h>

#include <TAxis.h>  // for TAxis
#include <TCanvas.h>
#include <TDatime.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <TPad.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TText.h>
#include <TMath.h>
#include <TPaveLabel.h>

#include <cstring>  // for memset
#include <ctime>
#include <fstream>
#include <iostream>  // for operator<<, basic_ostream, basic_os...
#include <sstream>
#include <vector>  // for vector

TpcMonDraw::TpcMonDraw(const std::string &name)
  : OnlMonDraw(name)
{
  // this TimeOffsetTicks is neccessary to get the time axis right
  TDatime T0(2003, 01, 01, 00, 00, 00);
  TimeOffsetTicks = T0.Convert();
  dbvars = new OnlMonDB(ThisName);
  return;
}

int TpcMonDraw::Init()
{
  return 0;
}

int TpcMonDraw::MakeCanvas(const std::string &name)
{
  OnlMonClient *cl = OnlMonClient::instance();
  int xsize = cl->GetDisplaySizeX();
  int ysize = cl->GetDisplaySizeY();
  if (name == "TpcMon1")
  {
    // xpos (-1) negative: do not draw menu bar
    TC[0] = new TCanvas(name.c_str(), "TpcMon Example Monitor", -1, 0, xsize / 2, ysize);
    // root is pathetic, whenever a new TCanvas is created root piles up
    // 6kb worth of X11 events which need to be cleared with
    // gSystem->ProcessEvents(), otherwise your process will grow and
    // grow and grow but will not show a definitely lost memory leak
    gSystem->ProcessEvents();
    Pad[0] = new TPad("tpcpad1", "who needs this?", 0.1, 0.5, 0.9, 0.9, 0);
    Pad[1] = new TPad("tpcpad2", "who needs this?", 0.1, 0.05, 0.9, 0.45, 0);
    Pad[0]->Draw();
    Pad[1]->Draw();
    // this one is used to plot the run number on the canvas
    transparent[0] = new TPad("transparent0", "this does not show", 0, 0, 1, 1);
    transparent[0]->SetFillStyle(4000);
    transparent[0]->Draw();
    TC[0]->SetEditable(false);
  }
  else if (name == "TpcMon2")
  {
    // xpos negative: do not draw menu bar
    TC[1] = new TCanvas(name.c_str(), "TpcMon2 Example Monitor", -xsize / 2, 0, xsize / 2, ysize);
    gSystem->ProcessEvents();
    Pad[2] = new TPad("tpcpad3", "who needs this?", 0.1, 0.5, 0.9, 0.9, 0);
    Pad[3] = new TPad("tpcpad4", "who needs this?", 0.1, 0.05, 0.9, 0.45, 0);
    Pad[2]->Draw();
    Pad[3]->Draw();
    // this one is used to plot the run number on the canvas
    transparent[1] = new TPad("transparent1", "this does not show", 0, 0, 1, 1);
    transparent[1]->SetFillStyle(4000);
    transparent[1]->Draw();
    TC[1]->SetEditable(false);
  }
  else if (name == "TpcMon3")
  {
    TC[2] = new TCanvas(name.c_str(), "TpcMon3 Example Monitor", xsize / 2, 0, xsize / 2, ysize);
    gSystem->ProcessEvents();
    Pad[4] = new TPad("tpcpad5", "who needs this?", 0.1, 0.5, 0.9, 0.9, 0);
    Pad[5] = new TPad("tpcpad6", "who needs this?", 0.1, 0.05, 0.9, 0.45, 0);
    Pad[4]->Draw();
    Pad[5]->Draw();
    // this one is used to plot the run number on the canvas
    //        transparent[2] = new TPad("transparent2", "this does not show", 0, 0, 1, 1);
    //        transparent[2]->SetFillStyle(4000);
    //        transparent[2]->Draw();
    //      TC[2]->SetEditable(0);
  }
  else if (name == "TPCModules")
  {
    TC[3] = new TCanvas(name.c_str(), "ADC Count by GEM Example", 1248, 598);
    TC[3]->Divide(2,1);
    TC[3]->SetEditable(false);
  }
  return 0;
}

int TpcMonDraw::Draw(const std::string &what)
{
  int iret = 0;
  int idraw = 0;
  if (what == "ALL" || what == "FIRST")
  {
    iret += DrawFirst(what);
    idraw++;
  }
  if (what == "ALL" || what == "SECOND")
  {
    iret += DrawSecond(what);
    idraw++;
  }
  if (what == "ALL" || what == "TPCMODULE")
  {
    iret += DrawTPCModules(what);
    idraw++;
  }
  if (what == "ALL" || what == "HISTORY")
  {
    iret += DrawHistory(what);
    idraw++;
  }
  if (!idraw)
  {
    std::cout << __PRETTY_FUNCTION__ << " Unimplemented Drawing option: " << what << std::endl;
    iret = -1;
  }
  return iret;
}

int TpcMonDraw::DrawFirst(const std::string & /* what */)
{
  OnlMonClient *cl = OnlMonClient::instance();
  TH1 *tpcmon_hist1 =  cl->getHisto("TPCMON_0","tpcmon_hist1");
  TH1 *tpcmon_hist2 =  cl->getHisto("TPCMON_0","tpcmon_hist1");
  if (!gROOT->FindObject("TpcMon1"))
  {
    MakeCanvas("TpcMon1");
  }
  TC[0]->SetEditable(true);
  TC[0]->Clear("D");
  Pad[0]->cd();
  if (tpcmon_hist1)
  {
    tpcmon_hist1->DrawCopy();
  }
  else
  {
    DrawDeadServer(transparent[0]);
    TC[0]->SetEditable(false);
    return -1;
  }
  Pad[1]->cd();
  if (tpcmon_hist2)
  {
    tpcmon_hist2->DrawCopy();
  }
  TText PrintRun;
  PrintRun.SetTextFont(62);
  PrintRun.SetTextSize(0.04);
  PrintRun.SetNDC();          // set to normalized coordinates
  PrintRun.SetTextAlign(23);  // center/top alignment
  std::ostringstream runnostream;
  std::string runstring;
  time_t evttime = cl->EventTime("CURRENT");
  // fill run number and event time into string
  runnostream << ThisName << "_1 Run " << cl->RunNumber()
              << ", Time: " << ctime(&evttime);
  runstring = runnostream.str();
  transparent[0]->cd();
  PrintRun.DrawText(0.5, 1., runstring.c_str());
  TC[0]->Update();
  TC[0]->Show();
  TC[0]->SetEditable(false);
  return 0;
}

int TpcMonDraw::DrawSecond(const std::string & /* what */)
{
  OnlMonClient *cl = OnlMonClient::instance();
  TH1 *tpcmon_hist1 =  cl->getHisto("TPCMON_0","tpcmon_hist2");
  TH1 *tpcmon_hist2 =  cl->getHisto("TPCMON_0","tpcmon_hist2");
  if (!gROOT->FindObject("TpcMon2"))
  {
    MakeCanvas("TpcMon2");
  }
  TC[1]->SetEditable(true);
  TC[1]->Clear("D");
  Pad[2]->cd();
  if (tpcmon_hist1)
  {
    tpcmon_hist1->DrawCopy();
  }
  else
  {
    DrawDeadServer(transparent[1]);
    TC[1]->SetEditable(false);
    return -1;
  }
  Pad[3]->cd();
  if (tpcmon_hist2)
  {
    tpcmon_hist2->DrawCopy();
  }
  TText PrintRun;
  PrintRun.SetTextFont(62);
  PrintRun.SetTextSize(0.04);
  PrintRun.SetNDC();          // set to normalized coordinates
  PrintRun.SetTextAlign(23);  // center/top alignment
  std::ostringstream runnostream;
  std::string runstring;
  time_t evttime = cl->EventTime("CURRENT");
  // fill run number and event time into string
  runnostream << ThisName << "_2 Run " << cl->RunNumber()
              << ", Time: " << ctime(&evttime);
  runstring = runnostream.str();
  transparent[1]->cd();
  PrintRun.DrawText(0.5, 1., runstring.c_str());
  TC[1]->Update();
  TC[1]->Show();
  TC[1]->SetEditable(false);
  return 0;
}

int TpcMonDraw::DrawTPCModules(const std::string & /* what */)
{
  std::cout<<"This is Charles' temporary function 02.15.23 !!!!!"<<std::endl;
  OnlMonClient *cl = OnlMonClient::instance();
  TH2 *tpcmon_NSIDEADC = (TH2*) cl->getHisto("TPCMON_0","NorthSideADC");
  TH2 *tpcmon_SSIDEADC = (TH2*) cl->getHisto("TPCMON_0","SouthSideADC");

  if (!gROOT->FindObject("TPCModules"))
  {
    MakeCanvas("TPCModules");
  }

  dummy_his1 = new TH2F("dummy_his1", "ADC Counts North Side", 100, -1.5, 1.5, 100, -1.5, 1.5); //dummy histos for titles
  dummy_his2 = new TH2F("dummy_his2", "ADC Counts South Side", 100, -1.5, 1.5, 100, -1.5, 1.5);

  //labels
  NS18 = new TPaveLabel( 1.046586,-0.1938999,1.407997,0.2144871, "18" );
  NS17 = new TPaveLabel( 0.962076,0.4382608,1.323487,0.8466479 , "17" );
  NS16 = new TPaveLabel( 0.4801947,0.8802139,0.8416056,1.288601 , "16" );
  NS15 = new TPaveLabel( -0.1823921,1.011681,0.1790189,1.425662, "15" );
  NS14 = new TPaveLabel( -0.8449788,0.8690253,-0.4835679,1.288601 , "14" );
  NS13 = new TPaveLabel( -1.30879,0.441058,-0.9473786,0.8550394 , "13" );
  NS12 = new TPaveLabel( -1.411009,-0.2050886,-1.049598,0.2144871, "12" );
  NS23 = new TPaveLabel( -1.302585,-0.7757116,-0.9471979,-0.3561359 , "23" );
  NS22 = new TPaveLabel( -0.8449788,-1.309971,-0.4835679,-0.8848013 , "22" );
  NS21 = new TPaveLabel( -0.1823921,-1.426557,0.1790189,-1.006982 , "21" );
  NS20 = new TPaveLabel( 0.4801947,-1.309076,0.8416056,-0.8839062 , "20" );
  NS19 = new TPaveLabel( 0.9622567,-0.7785088,1.323668,-0.3533387 , "19" );

  SS00 = new TPaveLabel( 1.046586,-0.1938999,1.407997,0.2144871, "00" );
  SS01 = new TPaveLabel( 0.962076,0.4382608,1.323487,0.8466479 , "01" );
  SS02 = new TPaveLabel( 0.4801947,0.8802139,0.8416056,1.288601 , "02" );
  SS03 = new TPaveLabel( -0.1823921,1.011681,0.1790189,1.425662, "03" );
  SS04 = new TPaveLabel( -0.8449788,0.8690253,-0.4835679,1.288601 , "04" );
  SS05 = new TPaveLabel( -1.30879,0.441058,-0.9473786,0.8550394 , "05" );
  SS06 = new TPaveLabel( -1.411009,-0.2050886,-1.049598,0.2144871, "06" );
  SS07 = new TPaveLabel( -1.302585,-0.7757116,-0.9471979,-0.3561359 , "07" );
  SS08 = new TPaveLabel( -0.8449788,-1.309971,-0.4835679,-0.8848013 , "08" );
  SS09 = new TPaveLabel( -0.1823921,-1.426557,0.1790189,-1.006982 , "09" );
  SS10 = new TPaveLabel( 0.4801947,-1.309076,0.8416056,-0.8839062 , "10" );
  SS11 = new TPaveLabel( 0.9622567,-0.7785088,1.323668,-0.3533387 , "11" );

  NS18->SetFillColor(0);
  NS17->SetFillColor(0);
  NS16->SetFillColor(0);
  NS15->SetFillColor(0);
  NS14->SetFillColor(0);
  NS13->SetFillColor(0);
  NS12->SetFillColor(0);
  NS23->SetFillColor(0);
  NS22->SetFillColor(0);
  NS21->SetFillColor(0);
  NS20->SetFillColor(0);
  NS19->SetFillColor(0);

  SS00->SetFillColor(0);
  SS01->SetFillColor(0);
  SS02->SetFillColor(0);
  SS03->SetFillColor(0);
  SS04->SetFillColor(0);
  SS05->SetFillColor(0);
  SS06->SetFillColor(0);
  SS07->SetFillColor(0);
  SS08->SetFillColor(0);
  SS09->SetFillColor(0);
  SS10->SetFillColor(0);
  SS11->SetFillColor(0);


  TC[3]->SetEditable(true);
  TC[3]->Clear("D");
  TC[3]->cd(1);
  dummy_his1->Draw("");
  if (tpcmon_NSIDEADC)
  {
    //std::cout<<"Yes, there is a histogram to draw !!!! Charles 02.16.23"<<std::endl;

    tpcmon_NSIDEADC->DrawCopy("colpolzsame");
  }
  SS00->Draw("same");
  SS01->Draw("same");
  SS02->Draw("same");
  SS03->Draw("same");
  SS04->Draw("same");
  SS05->Draw("same");
  SS06->Draw("same");
  SS07->Draw("same");
  SS08->Draw("same");
  SS09->Draw("same");
  SS10->Draw("same");
  SS11->Draw("same");

  TC[3]->cd(2);
  dummy_his2->Draw("");
  if (tpcmon_SSIDEADC)
  {
    tpcmon_SSIDEADC->DrawCopy("colpolzsame");
  }
  NS18->Draw("same");
  NS17->Draw("same");
  NS16->Draw("same");
  NS15->Draw("same");
  NS14->Draw("same");
  NS13->Draw("same");
  NS12->Draw("same");
  NS23->Draw("same");
  NS22->Draw("same");
  NS21->Draw("same");
  NS20->Draw("same");
  NS19->Draw("same");

  TC[3]->Update();

  //turn off stats box
  dummy_his1->SetStats(0);
  dummy_his2->SetStats(0);

  //dynamically set heat map color scale to start at the minimum and end at the maximum
  dummy_his1->SetMaximum(TMath::Max(tpcmon_SSIDEADC->GetBinContent(tpcmon_SSIDEADC->GetMaximumBin()),tpcmon_NSIDEADC->GetBinContent(tpcmon_NSIDEADC->GetMaximumBin())));
  dummy_his1->SetMinimum(TMath::Min(tpcmon_SSIDEADC->GetBinContent(tpcmon_SSIDEADC->GetMinimumBin()),tpcmon_NSIDEADC->GetBinContent(tpcmon_NSIDEADC->GetMinimumBin())));

  dummy_his2->SetMaximum(TMath::Max(tpcmon_SSIDEADC->GetBinContent(tpcmon_SSIDEADC->GetMaximumBin()),tpcmon_NSIDEADC->GetBinContent(tpcmon_NSIDEADC->GetMaximumBin())));
  dummy_his2->SetMinimum(TMath::Min(tpcmon_SSIDEADC->GetBinContent(tpcmon_SSIDEADC->GetMinimumBin()),tpcmon_NSIDEADC->GetBinContent(tpcmon_NSIDEADC->GetMinimumBin())));

  TC[3]->Show();
  TC[3]->SetEditable(false);
  
  return 0;
}

int TpcMonDraw::MakePS(const std::string &what)
{
  OnlMonClient *cl = OnlMonClient::instance();
  std::ostringstream filename;
  int iret = Draw(what);
  if (iret)  // on error no ps files please
  {
    return iret;
  }
  filename << ThisName << "_1_" << cl->RunNumber() << ".ps";
  TC[0]->Print(filename.str().c_str());
  filename.str("");
  filename << ThisName << "_2_" << cl->RunNumber() << ".ps";
  TC[1]->Print(filename.str().c_str());
  return 0;
}


int TpcMonDraw::MakeHtml(const std::string &what)
{
  int iret = Draw(what);
  if (iret)  // on error no html output please
  {
    return iret;
  }

  OnlMonClient *cl = OnlMonClient::instance();

  // Register the 1st canvas png file to the menu and produces the png file.
  std::string pngfile = cl->htmlRegisterPage(*this, "First Canvas", "1", "png");
  cl->CanvasToPng(TC[0], pngfile);

  // idem for 2nd canvas.
  pngfile = cl->htmlRegisterPage(*this, "Second Canvas", "2", "png");
  cl->CanvasToPng(TC[1], pngfile);
  // Now register also EXPERTS html pages, under the EXPERTS subfolder.

  std::string logfile = cl->htmlRegisterPage(*this, "EXPERTS/Log", "log", "html");
  std::ofstream out(logfile.c_str());
  out << "<HTML><HEAD><TITLE>Log file for run " << cl->RunNumber()
      << "</TITLE></HEAD>" << std::endl;
  out << "<P>Some log file output would go here." << std::endl;
  out.close();

  std::string status = cl->htmlRegisterPage(*this, "EXPERTS/Status", "status", "html");
  std::ofstream out2(status.c_str());
  out2 << "<HTML><HEAD><TITLE>Status file for run " << cl->RunNumber()
       << "</TITLE></HEAD>" << std::endl;
  out2 << "<P>Some status output would go here." << std::endl;
  out2.close();
  cl->SaveLogFile(*this);
  return 0;
}

int TpcMonDraw::DrawHistory(const std::string & /* what */)
{
  int iret = 0;
  // you need to provide the following vectors
  // which are filled from the db
  std::vector<float> var;
  std::vector<float> varerr;
  std::vector<time_t> timestamp;
  std::vector<int> runnumber;
  std::string varname = "tpcmondummy";
  // this sets the time range from whihc values should be returned
  time_t begin = 0;            // begin of time (1.1.1970)
  time_t end = time(nullptr);  // current time (right NOW)
  iret = dbvars->GetVar(begin, end, varname, timestamp, runnumber, var, varerr);
  if (iret)
  {
    std::cout << __PRETTY_FUNCTION__ << " Error in db access" << std::endl;
    return iret;
  }
  if (!gROOT->FindObject("TpcMon3"))
  {
    MakeCanvas("TpcMon3");
  }
  // timestamps come sorted in ascending order
  float *x = new float[var.size()];
  float *y = new float[var.size()];
  float *ex = new float[var.size()];
  float *ey = new float[var.size()];
  int n = var.size();
  for (unsigned int i = 0; i < var.size(); i++)
  {
    //       std::cout << "timestamp: " << ctime(&timestamp[i])
    // 	   << ", run: " << runnumber[i]
    // 	   << ", var: " << var[i]
    // 	   << ", varerr: " << varerr[i]
    // 	   << std::endl;
    x[i] = timestamp[i] - TimeOffsetTicks;
    y[i] = var[i];
    ex[i] = 0;
    ey[i] = varerr[i];
  }
  Pad[4]->cd();
  if (gr[0])
  {
    delete gr[0];
  }
  gr[0] = new TGraphErrors(n, x, y, ex, ey);
  gr[0]->SetMarkerColor(4);
  gr[0]->SetMarkerStyle(21);
  gr[0]->Draw("ALP");
  gr[0]->GetXaxis()->SetTimeDisplay(1);
  gr[0]->GetXaxis()->SetLabelSize(0.03);
  // the x axis labeling looks like crap
  // please help me with this, the SetNdivisions
  // don't do the trick
  gr[0]->GetXaxis()->SetNdivisions(-1006);
  gr[0]->GetXaxis()->SetTimeOffset(TimeOffsetTicks);
  gr[0]->GetXaxis()->SetTimeFormat("%Y/%m/%d %H:%M");
  delete[] x;
  delete[] y;
  delete[] ex;
  delete[] ey;

  varname = "tpcmoncount";
  iret = dbvars->GetVar(begin, end, varname, timestamp, runnumber, var, varerr);
  if (iret)
  {
    std::cout << __PRETTY_FUNCTION__ << " Error in db access" << std::endl;
    return iret;
  }
  x = new float[var.size()];
  y = new float[var.size()];
  ex = new float[var.size()];
  ey = new float[var.size()];
  n = var.size();
  for (unsigned int i = 0; i < var.size(); i++)
  {
    //       std::cout << "timestamp: " << ctime(&timestamp[i])
    // 	   << ", run: " << runnumber[i]
    // 	   << ", var: " << var[i]
    // 	   << ", varerr: " << varerr[i]
    // 	   << std::endl;
    x[i] = timestamp[i] - TimeOffsetTicks;
    y[i] = var[i];
    ex[i] = 0;
    ey[i] = varerr[i];
  }
  Pad[5]->cd();
  if (gr[1])
  {
    delete gr[1];
  }
  gr[1] = new TGraphErrors(n, x, y, ex, ey);
  gr[1]->SetMarkerColor(4);
  gr[1]->SetMarkerStyle(21);
  gr[1]->Draw("ALP");
  gr[1]->GetXaxis()->SetTimeDisplay(1);
  // TC[2]->Update();
  //    h1->GetXaxis()->SetTimeDisplay(1);
  //    h1->GetXaxis()->SetLabelSize(0.03);
  gr[1]->GetXaxis()->SetLabelSize(0.03);
  gr[1]->GetXaxis()->SetTimeOffset(TimeOffsetTicks);
  gr[1]->GetXaxis()->SetTimeFormat("%Y/%m/%d %H:%M");
  //    h1->Draw();
  delete[] x;
  delete[] y;
  delete[] ex;
  delete[] ey;

  TC[2]->Update();
  return 0;
}
