/*
 * Start ROOT,
 * .x pixieNet.C
 * "Confugure" button to initialize the device
 * "ADC" to acquire and display traces for checked channels.
 * "MCA" to start 5-sec MCA run and display checked channels.
 * "Exit" quits ROOT.
 */
#include <TGClient.h>
#include <TGButton.h>
#include <TH1.h>

#include "PixieNetDefs.h"

class MainFrame : public TGMainFrame {
private: 
    TGTextButton *btnConfigure, *btnADC, *btnMCA, *test0, *test1, *exit;
    TGCheckButton *chkAdc0, *chkAdc1, *chkAdc2, *chkAdc3; // Display ADC traces
    TGCheckButton *chkMca0, *chkMca1, *chkMca2, *chkMca3; // Display MCA
    TRootEmbeddedCanvas *fPlot;
    TCanvas *myc;
    
public:
    MainFrame(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~MainFrame();
    
    int Configure();
    int GetTraces();
    int ShowTraces();
    int GetMCA();
    int ShowMCA();
    
    void Test();
    
    ClassDef(MainFrame, 0)
};

MainFrame::MainFrame(const TGWindow *p, UInt_t w, UInt_t h) :
    TGMainFrame(p, w, h)
{
        TGCompositeFrame *chframeMain = new TGCompositeFrame(this, 600, 350, kHorizontalFrame | kFixedWidth);
        TGCompositeFrame *chframeAux  = new TGCompositeFrame(this, 600, 50, kHorizontalFrame | kFixedWidth);
        TGCompositeFrame *cvframeBtns  = new TGCompositeFrame(this, 150, 350, kVerticalFrame | kFixedWidth);
        TGGroupFrame *gvframeTraces = new TGGroupFrame(this, "ADC" , kVerticalFrame);
        TGGroupFrame *gvframeMCA = new TGGroupFrame(this, "MCA" , kVerticalFrame);
        TGGroupFrame *gvframeTest = new TGGroupFrame(this, "Test" , kVerticalFrame);
        
        btnConfigure = new TGTextButton(cvframeBtns, "Configure");
        btnADC = new TGTextButton(gvframeTraces, "Get Traces");
        chkAdc0 = new TGCheckButton(gvframeTraces, "Show Ch0");
        chkAdc1 = new TGCheckButton(gvframeTraces, "Show Ch1");
        chkAdc2 = new TGCheckButton(gvframeTraces, "Show Ch2");
        chkAdc3 = new TGCheckButton(gvframeTraces, "Show Ch3");
        
        btnMCA = new TGTextButton(gvframeMCA, "Get MCA");
        chkMca0 = new TGCheckButton(gvframeMCA, "Show Ch0");
        chkMca1 = new TGCheckButton(gvframeMCA, "Show Ch1");
        chkMca2 = new TGCheckButton(gvframeMCA, "Show Ch2");
        chkMca3 = new TGCheckButton(gvframeMCA, "Show Ch3");
        
        Pixel_t red, green, blue, violet;
        gClient->GetColorByName("red", red);
        gClient->GetColorByName("green", green);
        gClient->GetColorByName("blue", blue);
        gClient->GetColorByName("violet", violet);
        
        chkAdc0->SetBackgroundColor(red);
        chkAdc1->SetBackgroundColor(green);
        chkAdc2->SetBackgroundColor(blue);
        chkAdc3->SetBackgroundColor(violet);
        
        chkMca0->SetBackgroundColor(red);
        chkMca1->SetBackgroundColor(green);
        chkMca2->SetBackgroundColor(blue);
        chkMca3->SetBackgroundColor(violet);
        
        
        test0 = new TGTextButton(gvframeTest, "Test0");
        test1 = new TGTextButton(gvframeTest, "Test1");
        
        exit = new TGTextButton(chframeAux, "Exit","gApplication->Terminate(0)");
        
        fPlot = new TRootEmbeddedCanvas(0, this, 400, 300);
        Int_t wid = fPlot->GetCanvasWindowId();
        myc = new TCanvas("Canvas", 10, 10, wid);
        fPlot->AdoptCanvas(myc);
        // myc->Connect()?
        
        btnConfigure->Connect("Clicked()", "MainFrame", this, "Configure()");        
        btnADC->Connect("Clicked()", "MainFrame", this, "GetTraces()");        
        btnMCA->Connect("Clicked()", "MainFrame", this, "GetMCA()");        
        test0->Connect("Clicked()", "MainFrame", this, "Test()");        
        test1->Connect("Clicked()", "MainFrame", this, "Test()");        
        
        chkAdc0->SetState(kTRUE);
        chkAdc1->SetState(kTRUE);
        chkAdc2->SetState(kTRUE);
        chkAdc3->SetState(kTRUE);
        
        chkMca0->SetState(kTRUE);
        chkMca1->SetState(kTRUE);
        chkMca2->SetState(kTRUE);
        chkMca3->SetState(kTRUE);
        
        // Force re-draw on changes
        chkAdc0->Connect("Clicked()", "MainFrame", this, "ShowTraces()");
        chkAdc1->Connect("Clicked()", "MainFrame", this, "ShowTraces()");
        chkAdc2->Connect("Clicked()", "MainFrame", this, "ShowTraces()");
        chkAdc3->Connect("Clicked()", "MainFrame", this, "ShowTraces()");
        
        chkMca0->Connect("Clicked()", "MainFrame", this, "ShowMCA()");
        chkMca1->Connect("Clicked()", "MainFrame", this, "ShowMCA()");
        chkMca2->Connect("Clicked()", "MainFrame", this, "ShowMCA()");
        chkMca3->Connect("Clicked()", "MainFrame", this, "ShowMCA()");   
        
        // Traces
        gvframeTraces->AddFrame(btnADC, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
        
        gvframeTraces->AddFrame(chkAdc0, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        gvframeTraces->AddFrame(chkAdc1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        gvframeTraces->AddFrame(chkAdc2, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        gvframeTraces->AddFrame(chkAdc3, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        
        // MCA
        gvframeMCA->AddFrame(btnMCA, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
        
        gvframeMCA->AddFrame(chkMca0, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        gvframeMCA->AddFrame(chkMca1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        gvframeMCA->AddFrame(chkMca2, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        gvframeMCA->AddFrame(chkMca3, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        
        // Test buttons
        gvframeTest->AddFrame(test0, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
        gvframeTest->AddFrame(test1, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
            
        // Useful DAQ buttons
        cvframeBtns->AddFrame(btnConfigure, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
        cvframeBtns->AddFrame(gvframeTraces,       new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
        cvframeBtns->AddFrame(gvframeMCA,       new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
        //        Not displaying test buttons for now, commented out
//        cvframeBtns->AddFrame(gvframeTest,  new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
        
        chframeMain->AddFrame(cvframeBtns, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));
        chframeMain->AddFrame(fPlot, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 0, 0, 1, 1));
        
        chframeAux->AddFrame(exit, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 2));
        
        AddFrame(chframeMain, new TGLayoutHints(kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 2, 2, 5, 1));
        AddFrame(chframeAux,  new TGLayoutHints(kLHintsCenterX, 2, 2, 5, 1));
        
        SetWindowName("PixieNet control");
        
        SetWMSizeHints(640, 400, 1024, 640, 1, 1);
        MapSubwindows();
        Resize(GetDefaultSize());
        MapWindow();
        
}
    
MainFrame::~MainFrame()
{
    Cleanup();
}
int MainFrame::Configure()
{
    int retval = gSystem->Exec("./progfippi");
    if (retval) {
        fprintf(stderr, "*ERROR* ./progfippi failed\n");
        return(1);
    }
    retval = gSystem->Exec("./findsettings");
    if (retval) {
        fprintf(stderr, "*ERROR* ./findsettings failed\n");
        return(1);
    }  
    
    printf("*INFO* FPGA configured successfully\n");
    return(0);
}

int MainFrame::GetTraces()
{
   int retval = gSystem->Exec("./gettraces"); 
    if (retval) {
        fprintf(stderr, "*ERROR* acquire traces failed\n");
//        return(1);
    }
    // gettraces writes first line as a column descriptor, thus skipping first line
    gSystem->Exec("cat ADC.csv | tail -n +2 >  ADCchannels.csv");    
    
    ShowTraces();
    
    return(0);
}

int MainFrame::ShowTraces()
{
    printf("*INFO* Display ADC traces, %d samples\n", NTRACE_SAMPLES);
    
    // In order to avoid memory leaks with h, delete if was already drawn
    if (gROOT->FindObject("ADC traces")) {
        gROOT->FindObject("ADC traces")->Delete();
    }
    TH1F *h = new TH1F("ADC traces", "ADC traces", NTRACE_SAMPLES, 0, NTRACE_SAMPLES);
    h->GetXaxis()->SetTitle("sample");
    h->SetStats(kFALSE);

    // Use correct file name! Selecting sample number and ch0--ch3 value, comma-separated
    TGraph *g0 = new TGraph("ADCchannels.csv", "%lg %lg %*lg %*lg %*lg", ",");
    TGraph *g1 = new TGraph("ADCchannels.csv", "%lg %*lg %lg %*lg %*lg", ",");
    TGraph *g2 = new TGraph("ADCchannels.csv", "%lg %*lg %*lg %lg %*lg", ",");
    TGraph *g3 = new TGraph("ADCchannels.csv", "%lg %*lg %*lg %*lg %lg", ",");
    
    g0->SetName("adc0");
    g1->SetName("adc1");
    g2->SetName("adc2");
    g3->SetName("adc3");
    
    g0->SetLineColor(kRed);
    g1->SetLineColor(kGreen);
    g2->SetLineColor(kBlue);
    g3->SetLineColor(kViolet);
    
    // Set maximum and minimum for display
    // Scale using only checked channels
    Double_t traceMax = 0;
    Double_t traceMin = 65536;
    if (chkAdc0->IsOn()) {
       if (TMath::MaxElement(NTRACE_SAMPLES, g0->GetY()) > traceMax) traceMax = TMath::MaxElement(NTRACE_SAMPLES, g0->GetY());
       if (TMath::MinElement(NTRACE_SAMPLES, g0->GetY()) < traceMin) traceMin = TMath::MinElement(NTRACE_SAMPLES, g0->GetY());
    }
    
    if (chkAdc1->IsOn()) {
        if (TMath::MaxElement(NTRACE_SAMPLES, g1->GetY()) > traceMax) traceMax = TMath::MaxElement(NTRACE_SAMPLES, g1->GetY());
        if (TMath::MinElement(NTRACE_SAMPLES, g1->GetY()) < traceMin) traceMin = TMath::MinElement(NTRACE_SAMPLES, g1->GetY());
    }
    
    if (chkAdc2->IsOn()) {
        if (TMath::MaxElement(NTRACE_SAMPLES, g2->GetY()) > traceMax) traceMax = TMath::MaxElement(NTRACE_SAMPLES, g2->GetY());
        if (TMath::MinElement(NTRACE_SAMPLES, g2->GetY()) < traceMin) traceMin = TMath::MinElement(NTRACE_SAMPLES, g2->GetY());
    }
    
    if (chkAdc3->IsOn()) {
        if (TMath::MaxElement(NTRACE_SAMPLES, g3->GetY()) > traceMax) traceMax = TMath::MaxElement(NTRACE_SAMPLES, g3->GetY());
        if (TMath::MinElement(NTRACE_SAMPLES, g3->GetY()) < traceMin) traceMin = TMath::MinElement(NTRACE_SAMPLES, g3->GetY());
    }
    
    h->SetAxisRange(0, NTRACE_SAMPLES,"X");
    h->GetYaxis()->SetRangeUser(traceMin, traceMax);
   
    myc->cd();
    h->Draw();  
    
    // Draw only checked channels
    if (chkAdc0->IsOn()) g0->Draw("L");
    if (chkAdc1->IsOn()) g1->Draw("L");
    if (chkAdc2->IsOn()) g2->Draw("L");
    if (chkAdc3->IsOn()) g3->Draw("L");
    
    myc->Update();
    
    return(0);
}

int MainFrame::GetMCA()
{
    myc->Clear();
    
    int retval = gSystem->Exec("./startdaq"); 
    if (retval) {
        fprintf(stderr, "*ERROR* acquire spectra failed\n");
//        return(1);
    }
    // gettraces writes first line as a column descriptor, thus skipping first line
    gSystem->Exec("cat MCA.csv | tail -n +2 >  MCAchannels.csv");    
    
    ShowMCA();
    
    return(0);
}

int MainFrame::ShowMCA()
{
    printf("Display MCA, %d bins\n", MAX_MCA_BINS);
    // In order to avoid memory leaks with h, delete if was already drawn
    if (gROOT->FindObject("MCA")) {
        gROOT->FindObject("MCA")->Delete();
    }
    TH1F *h = new TH1F("MCA", "MCA", WEB_MCA_BINS, 0, MAX_MCA_BINS);
    h->GetXaxis()->SetTitle("energy");
    h->SetStats(kFALSE);

    // Use correct file name! Selecting sample number and ch0--ch3 value, comma-separated
    TGraph *g0 = new TGraph("MCAchannels.csv", "%lg %lg %*lg %*lg %*lg", ",");
    TGraph *g1 = new TGraph("MCAchannels.csv", "%lg %*lg %lg %*lg %*lg", ",");
    TGraph *g2 = new TGraph("MCAchannels.csv", "%lg %*lg %*lg %lg %*lg", ",");
    TGraph *g3 = new TGraph("MCAchannels.csv", "%lg %*lg %*lg %*lg %lg", ",");
    
    g0->SetName("mca0");
    g1->SetName("mca1");
    g2->SetName("mca2");
    g3->SetName("mca3");
    
    g0->SetLineColor(kRed);
    g1->SetLineColor(kGreen);
    g2->SetLineColor(kBlue);
    g3->SetLineColor(kViolet);
    
    // Set maximum and minimum for display
    // Scale using only checked channels
    
    Double_t mcaMax = 0;
    Double_t mcaMin = 65536;
    if (chkMca0->IsOn()) {
       if (TMath::MaxElement(MAX_MCA_BINS, g0->GetY()) > mcaMax) mcaMax = TMath::MaxElement(MAX_MCA_BINS, g0->GetY());
       if (TMath::MinElement(MAX_MCA_BINS, g0->GetY()) < mcaMin) mcaMin = TMath::MinElement(MAX_MCA_BINS, g0->GetY());
    }
    
    if (chkMca1->IsOn()) {
        if (TMath::MaxElement(MAX_MCA_BINS, g1->GetY()) > mcaMax) mcaMax = TMath::MaxElement(MAX_MCA_BINS, g1->GetY());
        if (TMath::MinElement(MAX_MCA_BINS, g1->GetY()) < mcaMin) mcaMin = TMath::MinElement(MAX_MCA_BINS, g1->GetY());
    }
    
    if (chkMca2->IsOn()) {
        if (TMath::MaxElement(MAX_MCA_BINS, g2->GetY()) > mcaMax) mcaMax = TMath::MaxElement(MAX_MCA_BINS, g2->GetY());
        if (TMath::MinElement(MAX_MCA_BINS, g2->GetY()) < mcaMin) mcaMin = TMath::MinElement(MAX_MCA_BINS, g2->GetY());
    }
    
    if (chkMca3->IsOn()) {
        if (TMath::MaxElement(MAX_MCA_BINS, g3->GetY()) > mcaMax) mcaMax = TMath::MaxElement(MAX_MCA_BINS, g3->GetY());
        if (TMath::MinElement(MAX_MCA_BINS, g3->GetY()) < mcaMin) mcaMin = TMath::MinElement(MAX_MCA_BINS, g3->GetY());
    }
    
    
    h->SetAxisRange(0, MAX_MCA_BINS,"X");
    if (myc->GetLogy()) h->GetYaxis()->SetRangeUser(0.1, mcaMax);
    else  h->GetYaxis()->SetRangeUser(mcaMin, mcaMax);
   
    myc->cd();
    h->Draw();  
    
    // Draw only checked channels
    if (chkMca0->IsOn()) g0->Draw("L");
    if (chkMca1->IsOn()) g1->Draw("L");
    if (chkMca2->IsOn()) g2->Draw("L");
    if (chkMca3->IsOn()) g3->Draw("L");
    
    myc->Update();    
    return(0);
}
    
void MainFrame::Test()
{
    printf("*INFO* Clicked test button, groupped in GroupFrame\n");
}

void pixieNet()
{
    new MainFrame(gClient->GetRoot(), 640, 400);
}
