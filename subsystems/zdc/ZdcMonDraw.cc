#include "ZdcMonDraw.h"

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

#include <cstring>  // for memset
#include <ctime>
#include <fstream>
#include <iostream>  // for operator<<, basic_ostream, basic_os...
#include <sstream>
#include <vector>  // for vector

ZdcMonDraw::ZdcMonDraw(const std::string &name)
  : OnlMonDraw(name)
{
  return;
}

int ZdcMonDraw::Init()
{
  return 0;
}

int ZdcMonDraw::MakeCanvas(const std::string &name)
{
  OnlMonClient *cl = OnlMonClient::instance();
  int xsize = cl->GetDisplaySizeX();
  int ysize = cl->GetDisplaySizeY();
  if (name == "ZdcMon1")
  {
    // xpos (-1) negative: do not draw menu bar
    TC[0] = new TCanvas(name.c_str(), "ZdcMon Example Monitor", -1, 0, xsize / 2, ysize);
    // root is pathetic, whenever a new TCanvas is created root piles up
    // 6kb worth of X11 events which need to be cleared with
    // gSystem->ProcessEvents(), otherwise your process will grow and
    // grow and grow but will not show a definitely lost memory leak
    gSystem->ProcessEvents();
    Pad[0] = new TPad("zdcpad1", "who needs this?", 0.05, 0.5, 0.5, 0.9, 0);
    Pad[1] = new TPad("zdcpad2", "who needs this?", 0.5, 0.5, 0.95, 0.9, 0);
    Pad[2] = new TPad("zdcpad3", "who needs this?", 0.05, 0.05, 0.5, 0.45, 0);
    Pad[3] = new TPad("zdcpad4", "who needs this?", 0.5, 0.05, 0.95, 0.45, 0);
    Pad[0]->Draw();
    Pad[1]->Draw();
    Pad[2]->Draw();
    Pad[3]->Draw();
    // this one is used to plot the run number on the canvas
    transparent[0] = new TPad("transparent0", "this does not show", 0, 0, 1, 1);
    transparent[0]->SetFillStyle(4000);
    transparent[0]->Draw();
    TC[0]->SetEditable(false);
  }
  else if (name == "ZdcMon2")
  {
    // xpos negative: do not draw menu bar
    TC[1] = new TCanvas(name.c_str(), "ZdcMon2 Example Monitor", -xsize / 2, 0, xsize / 2, ysize);
    gSystem->ProcessEvents();
    Pad[4] = new TPad("zdcpad3", "who needs this?", 0.1, 0.5, 0.9, 0.9, 0);
    Pad[5] = new TPad("zdcpad4", "who needs this?", 0.1, 0.05, 0.9, 0.45, 0);
    Pad[4]->Draw();
    Pad[5]->Draw();
    // this one is used to plot the run number on the canvas
    transparent[1] = new TPad("transparent1", "this does not show", 0, 0, 1, 1);
    transparent[1]->SetFillStyle(4000);
    transparent[1]->Draw();
    TC[1]->SetEditable(false);
  }
  // DRAW SMD GOOD AND SMALL VALUES
  else if (name == "SmdValues")
  {
    // xpos negative: do not draw menu bar
    TC[2] = new TCanvas(name.c_str(), "Smd Values", 0 , -ysize / 2, xsize, ysize / 2);
    gSystem->ProcessEvents();
    Pad[6] = new TPad("Smd Value", "Smd Value", 0.05, 0.05, 0.35, 0.9, 0);
    Pad[7] = new TPad("Smd Value (good)", "Smd Value (good)", 0.35, 0.05, 0.65, 0.9, 0);
    Pad[8] = new TPad("Smd Value (small)", "Smd Value (small)", 0.65, 0.05, 0.95, 0.9, 0);

    Pad[6]->Draw();    
    Pad[7]->Draw();
    Pad[8]->Draw();

    // this one is used to plot the run number on the canvas
    transparent[2] = new TPad("transparent1", "this does not show", 0, 0, 1, 1);
    transparent[2]->SetFillStyle(4000);
    transparent[2]->Draw();
    TC[2]->SetEditable(false);
  }

  // DRAW SMD NORTH AND SOUTH
  else if (name == "SmdNorthandSouth")
  {
    // xpos negative: do not draw menu bar
    TC[3] = new TCanvas(name.c_str(), "Smd North and South", -xsize*0.9, -ysize*0.9, xsize*0.9, ysize*0.9);
    gSystem->ProcessEvents();
    /*
    Pad[7]  = new TPad("Smd Ver North", "Smd Ver North", 0.02, 0.05, 0.26, 0.35, 0);
    Pad[8]  = new TPad("Smd Ver North (good)",  "Smd Ver North (good)", 0.26, 0.05, 0.5, 0.35, 0);
    Pad[9]  = new TPad("Smd Ver North (small)",  "Title 6", 0.5, 0.05, 0.74, 0.35, 0);
    Pad[10] = new TPad("Smd Hor North", "Smd Hor North", 0.74, 0.05, 0.98, 0.35, 0);

    Pad[11] = new TPad("Smd Hor North (good)", "Smd Hor North (good)", 0.02, 0.35, 0.26, 0.65, 0);
    Pad[12] = new TPad("Smd Hor North (small)", "Smd Hor North (small)", 0.26, 0.35, 0.5, 0.65, 0);
    Pad[13] = new TPad("Smd Ver South", "Smd Ver South", 0.5, 0.35, 0.74, 0.65, 0);
    Pad[14] = new TPad("Smd Hor South", "Smd Hor South", 0.74, 0.35, 0.98, 0.65, 0);

    Pad[15] = new TPad("Smd sum Ver North", "Smd sum Ver North", 0.02, 0.65, 0.26, 0.95, 0);
    Pad[16] = new TPad("Smd sum Hor North", "Smd sum Hor North", 0.26, 0.65, 0.5, 0.95, 0);
    Pad[17] = new TPad("Smd sum Ver South", "Smd sum Ver South", 0.5, 0.65, 0.74, 0.95, 0);
    Pad[18] = new TPad("Smd sum Hor South", "Smd sum Hor South", 0.74, 0.65, 0.98, 0.95, 0);  
    */

    Pad[9]  = new TPad("Smd Ver North (good)",  "Smd Ver North (good)", 0.02, 0.05, 0.26, 0.35, 0);
    Pad[10]  = new TPad("Smd Hor North (good)", "Smd Hor North (good)", 0.26, 0.05, 0.5, 0.35, 0);
    Pad[11]  = new TPad("Smd Ver North (small)",  "Smd Ver North (small)", 0.5, 0.05, 0.74, 0.35, 0);
    Pad[12] = new TPad("Smd Hor North (small)", "Smd Hor North (small)", 0.74, 0.05, 0.98, 0.35, 0);

    Pad[13]  = new TPad("Smd Ver North", "Smd Ver North", 0.02, 0.35, 0.26, 0.65, 0);
    Pad[14] = new TPad("Smd Hor North", "Smd Hor North", 0.26, 0.35, 0.5, 0.65, 0);
    Pad[15] = new TPad("Smd Ver South", "Smd Ver South", 0.5, 0.35, 0.74, 0.65, 0);
    Pad[16] = new TPad("Smd Hor South", "Smd Hor South", 0.74, 0.35, 0.98, 0.65, 0);

    Pad[17] = new TPad("Smd sum Ver North", "Smd sum Ver North", 0.02, 0.65, 0.26, 0.95, 0);
    Pad[18] = new TPad("Smd sum Hor North", "Smd sum Hor North", 0.26, 0.65, 0.5, 0.95, 0);
    Pad[19] = new TPad("Smd sum Ver South", "Smd sum Ver South", 0.5, 0.65, 0.74, 0.95, 0);
    Pad[20] = new TPad("Smd sum Hor South", "Smd sum Hor South", 0.74, 0.65, 0.98, 0.95, 0); 

    Pad[9]->Draw();
    Pad[10]->Draw();
    Pad[11]->Draw();
    Pad[12]->Draw();
    Pad[13]->Draw();
    Pad[14]->Draw();
    Pad[15]->Draw();
    Pad[16]->Draw();
    Pad[17]->Draw();
    Pad[18]->Draw();
    Pad[19]->Draw();
    Pad[20]->Draw();
    // this one is used to plot the run number on the canvas
    transparent[3] = new TPad("transparent1", "this does not show", 0, 0, 1, 1);
    transparent[3]->SetFillStyle(4000);
    transparent[3]->Draw();
    TC[3]->SetEditable(false);
  }

  return 0;
}

int ZdcMonDraw::Draw(const std::string &what)
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
  
  if (what == "ALL" || what == "SMDVALUES")
  {
    iret += DrawSmdValues(what);
    idraw++;
  }
  
  if (what == "ALL" || what == "SMDN&S")
  {
    iret += DrawSmdNorthandSouth(what);
    idraw++;
  }

  if (!idraw)
  {
    std::cout << __PRETTY_FUNCTION__ << " Unimplemented Drawing option: " << what << std::endl;
    iret = -1;
  }
  return iret;
}

int ZdcMonDraw::DrawFirst(const std::string & /* what */)
{
  OnlMonClient *cl = OnlMonClient::instance();
  TH1 *zdc_adc_south = cl->getHisto("ZDCMON_0","zdc_adc_south");
  TH1 *zdc_adc_north = cl->getHisto("ZDCMON_0","zdc_adc_north");
  TH2 *smd_xy_north = (TH2*) cl->getHisto("ZDCMON_0","smd_xy_north");
  TH2 *smd_xy_south = (TH2*) cl->getHisto("ZDCMON_0","smd_xy_south");

  if (!gROOT->FindObject("ZdcMon1"))
  {
    MakeCanvas("ZdcMon1");
  }
  TC[0]->SetEditable(true);
  TC[0]->Clear("D");
  Pad[0]->cd();
  gPad->SetLogy();
  gPad->SetLogx();
  if (zdc_adc_south)
  {
    zdc_adc_south->Scale(1/zdc_adc_south->Integral(), "width");
    zdc_adc_south->DrawCopy();
  }
  else
  {
    DrawDeadServer(transparent[0]);
    TC[0]->SetEditable(false);
    return -1;
  }
  Pad[1]->cd();
  gPad->SetLogy();
  gPad->SetLogx();
  if (zdc_adc_north)
  {
    zdc_adc_north->Scale(1/zdc_adc_north->Integral(), "width");
    zdc_adc_north->DrawCopy();
  }

  Pad[2]->cd();
  if (smd_xy_north){smd_xy_north->DrawCopy("colz");}
  Pad[3]->cd();
  if (smd_xy_south){smd_xy_south->DrawCopy("colz");}
  
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

int ZdcMonDraw::DrawSecond(const std::string & /* what */)
{
  OnlMonClient *cl = OnlMonClient::instance();
  TH1 *zdc_adc_south = cl->getHisto("ZDCMON_0","zdc_adc_south");
  TH1 *zdc_adc_north = cl->getHisto("ZDCMON_0","zdc_adc_north");
  
  if (!gROOT->FindObject("ZdcMon2"))
  {
    MakeCanvas("ZdcMon2");
  }
  TC[1]->SetEditable(true);
  TC[1]->Clear("D");
  Pad[4]->cd();
  if (zdc_adc_south)
  {
    zdc_adc_south->DrawCopy();
  }
  else
  {
    DrawDeadServer(transparent[1]);
    TC[1]->SetEditable(false);
    return -1;
  }
  Pad[5]->cd();
  if (zdc_adc_north)
  {
     zdc_adc_north->DrawCopy();
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

int ZdcMonDraw::DrawSmdValues(const std::string & /* what */)
{
  
  OnlMonClient *cl = OnlMonClient::instance();
  TH2 *smd_value = (TH2*) cl->getHisto("ZDCMON_0","smd_value");
  TH2 *smd_value_good = (TH2*) cl->getHisto("ZDCMON_0","smd_value_good");
  TH2 *smd_value_small = (TH2*) cl->getHisto("ZDCMON_0","smd_value_small");
  if (!gROOT->FindObject("SmdValues"))
  {
    MakeCanvas("SmdValues");
  }

  TC[2]->SetEditable(true);
  TC[2]->Clear("D");
  Pad[6]->cd();
  if (smd_value)
  {
    smd_value->DrawCopy();
  }
  else
  {
    DrawDeadServer(transparent[2]);
    TC[2]->SetEditable(false);
    return -1;
  }

  Pad[7]->cd();
  if (smd_value_good) {smd_value_good->DrawCopy();}
  Pad[8]->cd();
  if (smd_value_small) {smd_value_small->DrawCopy();}

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
  transparent[2]->cd();
  PrintRun.DrawText(0.5, 1., runstring.c_str());
  TC[2]->Update();
  TC[2]->Show();
  TC[2]->SetEditable(false);
  
  return 0;
}

int ZdcMonDraw::DrawSmdNorthandSouth(const std::string & /* what */)
{
  OnlMonClient *cl = OnlMonClient::instance();

  // get pointer for each histogram
  TH1 *smd_hor_south = cl->getHisto("ZDCMON_0", "smd_hor_south");
  TH1 *smd_ver_south = cl->getHisto("ZDCMON_0", "smd_ver_south");
  TH1 *smd_hor_north = cl->getHisto("ZDCMON_0", "smd_hor_north");
  TH1 *smd_ver_north = cl->getHisto("ZDCMON_0", "smd_ver_north");

  TH1 *smd_hor_north_small = cl->getHisto("ZDCMON_0", "smd_hor_north_small");
  TH1 *smd_ver_north_small = cl->getHisto("ZDCMON_0", "smd_ver_north_small");
  TH1 *smd_hor_north_good = cl->getHisto("ZDCMON_0", "smd_hor_north_good");
  TH1 *smd_ver_north_good = cl->getHisto("ZDCMON_0", "smd_ver_north_good");
  
  TH1 *smd_sum_hor_south = cl->getHisto("ZDCMON_0", "smd_sum_hor_south");
  TH1 *smd_sum_ver_south = cl->getHisto("ZDCMON_0", "smd_sum_ver_south");
  TH1 *smd_sum_hor_north = cl->getHisto("ZDCMON_0", "smd_sum_hor_north");
  TH1 *smd_sum_ver_north = cl->getHisto("ZDCMON_0", "smd_sum_ver_north");

  if (!gROOT->FindObject("SmdNorthandSouth"))
  {
    MakeCanvas("SmdNorthandSouth");
  }
  TC[3]->SetEditable(true);
  TC[3]->Clear("D");
  Pad[9]->cd();

  // VERTICAL AND HORIZONTAL NORTH (good and small)
  if (smd_ver_north_good)
  {
    smd_ver_north_good->DrawCopy();
    
  }
  else
  {
    DrawDeadServer(transparent[3]);
    TC[3]->SetEditable(false);
    return -1;
  }
  Pad[10]->cd();
  if (smd_hor_north_good) {smd_hor_north_good->DrawCopy();}
  Pad[11]->cd();
  if (smd_ver_north_small) {smd_ver_north_small->DrawCopy();}
  Pad[12]->cd();
  if (smd_hor_north_small) {smd_hor_north_small->DrawCopy();}

  // VERTICAL AND HORIZONTAL NORTH
  Pad[13]->cd();
  if (smd_ver_north){smd_ver_north->DrawCopy();}
  Pad[14]->cd();
  if (smd_hor_north){smd_hor_north->DrawCopy();}

  // VERTICAL AND HORIZONTAL SOUTH (good and small do not exist for south)
  Pad[15]->cd();
  if (smd_ver_south) {smd_ver_south->DrawCopy();}
  Pad[16]->cd();
  if (smd_hor_south) {smd_hor_south->DrawCopy();}

  // SUMS
  Pad[17]->cd();
  if (smd_sum_ver_north) {smd_sum_ver_north->DrawCopy();}
  Pad[18]->cd();
  if (smd_sum_hor_north) {smd_sum_hor_north->DrawCopy();}
  Pad[19]->cd();
  if (smd_sum_ver_south) {smd_sum_ver_south->DrawCopy();}
  Pad[20]->cd();
  if (smd_sum_hor_north) {smd_sum_hor_south->DrawCopy();}
 
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
  transparent[3]->cd();
  PrintRun.DrawText(0.5, 1., runstring.c_str());
  TC[3]->Update();
  TC[3]->Show();
  TC[3]->SetEditable(false);
  return 0;


}


int ZdcMonDraw::SavePlot(const std::string &what, const std::string &type)
{

  OnlMonClient *cl = OnlMonClient::instance();
  int iret = Draw(what);
  if (iret)  // on error no png files please
  {
    return iret;
  }
  int icnt = 0;
  for (TCanvas *canvas : TC)
  {
    if (canvas == nullptr)
    {
      continue;
    }
    icnt++;
    std::string filename = ThisName + "_" + std::to_string(icnt) + "_" +
      std::to_string(cl->RunNumber()) + "." + type;
    cl->CanvasToPng(canvas, filename);
  }
  return 0;
}

int ZdcMonDraw::MakeHtml(const std::string &what)
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
  // // idem for 3rd canvas.
  // pngfile = cl->htmlRegisterPage(*this, "Third Canvas", "3", "png");
  // cl->CanvasToPng(TC[2], pngfile);
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

  std::string smdvaluesplots = cl->htmlRegisterPage(*this, "EXPERTS/Log", "log", "html");
  std::ofstream out3(smdvaluesplots.c_str());
  out3 << "<HTML><HEAD><TITLE>Log file for run " << cl->RunNumber()
      << "</TITLE></HEAD>" << std::endl;
  out3 << "<P>Some SmdValues-related-output would go here." << std::endl;
  out3.close();

  std::string smdnorthandsouth = cl->htmlRegisterPage(*this, "EXPERTS/Log", "log", "html");
  std::ofstream out4(smdnorthandsouth.c_str());
  out4 << "<HTML><HEAD><TITLE>Log file for run " << cl->RunNumber()
      << "</TITLE></HEAD>" << std::endl;
  out4 << "<P>Some SmdNorthandSouth-related-output would go here." << std::endl;
  out4.close();


  return 0;
}
