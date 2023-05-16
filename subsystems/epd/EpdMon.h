#ifndef EPD_EPDMON_H
#define EPD_EPDMON_H

#include <onlmon/OnlMon.h>
#include <vector>
#include <TMath.h>

class CaloWaveformFitting;
class TowerInfoContainer;
class Event;
class OnlMonDB;
class TH1;
class TH2;
class Packet;
class runningMean;

class EpdMon : public OnlMon
{
 public:
  EpdMon(const std::string &name);
  virtual ~EpdMon();

  int process_event(Event *evt);
  int Init();
  int BeginRun(const int runno);
  int EpdMapChannel(int ch);
  int Reset();

 protected:
  std::vector<float> getSignal(Packet *p, const int channel);
  std::vector<float> anaWaveformFast(Packet *p, const int channel);
  std::vector<float> anaWaveformTemp(Packet *p, const int channel);
  int DBVarInit();
  int evtcnt = 0;
  int idummy = 0;
  
  const int Nsector = 24;
  const int Nchannel = 192*4;
  const int packetlow = 9001;
  const int packethigh = 9004;

  const int nChannels = 744;
  int nPhi0 = 12;
  int nPhi = 24;
  int nRad = 24;
  double axislimit = TMath::Pi();
  
  OnlMonDB *dbvars = nullptr;
  TH2 *h_ADC0_s = nullptr;
  TH2 *h_ADC0_n = nullptr;
  TH2 *h_ADC_s = nullptr;
  TH2 *h_ADC_n = nullptr;
  
  TH2 *h_hits0_s = nullptr;
  TH2 *h_hits0_n = nullptr;
  TH2 *h_hits_s = nullptr;
  TH2 *h_hits_n = nullptr;
  
  TH2 *h_ADC_corr = nullptr;
  TH2 *h_hits_corr = nullptr;
  
  TH1* h1_waveform_twrAvg = nullptr;
  TH1* h1_waveform_time = nullptr;
  TH1* h1_waveform_pedestal = nullptr;
  TH1* h_event = nullptr;

  TH1* h1_epd_fitting_sigDiff = nullptr;
  TH1* h1_epd_fitting_pedDiff = nullptr;
  TH1* h1_epd_fitting_timeDiff = nullptr;

  TH1* h_ADC_channel[744] = {nullptr};

  std::string runtypestr = "Unknown";
  std::string id_string;

  CaloWaveformFitting* WaveformProcessingFast = nullptr;
  CaloWaveformFitting* WaveformProcessingTemp = nullptr;

};


#endif /* EPD_EPDMON_H */
