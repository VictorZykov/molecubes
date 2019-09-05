/// Betajaen's GUI 015
/// Written by Robin "Betajaen" Southern 07-Nov-2006, http://www.ogre3d.org/wiki/index.php/BetaGUI
/// This code is under the Whatevar! licence. Do what you want; but keep the original copyright header.
/// This code is not meant to be readable, if you base your future source on this, I will laugh at you.

#ifndef BETAGUI_H
#define BETAGUI_H

//***************************************************************************************************
//***************************************************************************************************

#include "Ogre.h"
using namespace Ogre;using namespace std;namespace BetaGUI {class GUI;class Window;class Button;
class TextInput;class Callback;enum wt{NONE=0,MOVE=1,RESIZE=2,RESIZE_AND_MOVE=3};
class GUI{public:friend class Window;friend class Button;friend class TextInput;
	GUI(String font, uint fontSize);~GUI();	bool injectMouse(uint x, uint y, bool LMB);
	bool injectKey(String key, uint x, uint y);void injectBackspace(uint x, uint y){injectKey("!b",x,y);}
	Window* createWindow(Vector4,String,wt,String c="");void destroyWindow(Window *w){mXW=w;}
	OverlayContainer* createOverlay(String,Vector2,Vector2,String m="",String="",bool a=true);
	OverlayContainer* createMousePointer(Vector2 size,String material);protected:uint wc,bc,tc;
	Overlay* mO;vector<Window*>WN;Window *mXW;OverlayContainer* mMP;String mFont;uint mFontSize;};
class Window{public:friend class Button;friend class TextInput;friend class GUI;
	Button* createButton(Vector4 Dimensions, String Material, String Text, Callback callback);
	TextInput* createTextInput(Vector4 Dimensions, String Material, String Value, uint length);
	OverlayContainer* createStaticText(Vector4 Dimensions, String Text);
	void hide(){mO->hide();}void show(){mO->show();}bool isVisible(){return mO->isVisible();}
	protected:Window(Vector4 Dimensions, String Material, wt type, String caption, GUI *gui);~Window();
	bool check(uint x, uint y, bool LMB); bool checkKey(String key, uint x, uint y);TextInput* mATI;
	Button *mRZ,*mAB,*mTB;uint x,y,w,h;GUI *mGUI;OverlayContainer* mO;vector<BetaGUI::Button*>mB;vector<BetaGUI::TextInput*>mT;};
class BetaGUIListener{public:virtual void onButtonPress(Button *ref) = 0;};
class Callback{public:friend class Window;friend class Button;Callback(){t=0;}
	Callback(void(*functionPointer)(Button *ref)){t=1;fp=functionPointer;}
	Callback(BetaGUIListener *L){t=2;LS=L;}protected:uchar t;void(*fp)(Button *r);BetaGUIListener *LS;};
class Button{public:friend class Window;OverlayContainer* mO,*mCP;String mmn,mma;protected:
	Button(Vector4 Dimensions, String Material, String Text, Callback callback, Window *parent);
	~Button(){mO->getParent()->removeChild(mO->getName());mCP->getParent()->removeChild(mCP->getName());}
	void activate(bool a){if(!a&&mmn!="")mO->setMaterialName(mmn);if(a&&mma!="")mO->setMaterialName(mma);}
	bool in(uint mx,uint my,uint px,uint py) {return(!(mx>=x+px&&my>=y+py))||(!(mx<=x+px+w&&my<=y+py+h));}
	Callback callback;uint x,y,w,h;};
class TextInput{public: friend class Window;
	TextInput(Vector4 Dimensions, String Material, String Value, uint length, Window *parent);~TextInput(){}
	bool in(uint mx,uint my,uint px,uint py) {return(!(mx>=x+px&&my>=y+py))||(!(mx<=x+px+w&&my<=y+py+h));}
	OverlayContainer* mO,*mCP;String mmn,mma,value;uint x,y,w,h,length;
	public:String getValue(){return value;}void setValue(String v){mCP->setCaption(value=v);}};
}/*** Optional Seperate Code that goes into a .cpp part : Cut downwards and paste  ***/
using namespace Ogre;using namespace std;namespace BetaGUI {
GUI::GUI(String font, uint fontSize):mXW(0), mMP(0),mFont(font),mFontSize(fontSize),wc(0),bc(0),tc(0){
	mO=OverlayManager::getSingleton().create("BetaGUI");mO->show();}
GUI::~GUI(){for(uint i=0;i < WN.size();i++)delete WN[i];WN.clear();}
bool GUI::injectMouse(uint x,uint y,bool LMB){if(mMP)mMP->setPosition(x,y);
	if(mXW){for(vector<Window*>::iterator i=WN.begin();i!=WN.end();i++) {if(mXW==(*i)){delete mXW;WN.erase(i);mXW=0;return false;}}}
	for(uint i=0;i<WN.size();i++){if(WN[i]->check(x,y,LMB)){return true;}}return false;}
bool GUI::injectKey(String key, uint x,uint y){for(uint i=0;i<WN.size();i++){if(WN[i]->checkKey(key,x,y)){return true;}}return false;}
OverlayContainer* GUI::createOverlay(String N,Vector2 P,Vector2 D,String M,String C,bool A){String t="Panel";if (C!="")t="TextArea";
	OverlayContainer* e=static_cast<OverlayContainer*>(OverlayManager::getSingleton().createOverlayElement(t, N));
	e->setMetricsMode(GMM_PIXELS);e->setDimensions(D.x,D.y);e->setPosition(P.x,P.y);if (M!="")e->setMaterialName(M);
	if (C!=""){e->setCaption(C);e->setParameter("font_name",mFont);e->setParameter("char_height",StringConverter::toString(mFontSize));}
	if(A){mO->add2D(e);e->show();}return e;}
OverlayContainer* GUI::createMousePointer(Vector2 d, String m) {
	Overlay* o=OverlayManager::getSingleton().create("BetaGUI.MP");o->setZOrder(500);mMP=createOverlay("bg.mp",Vector2(0,0),d,m,"", false);
	o->add2D(mMP);o->show();mMP->show();return mMP;}
Window* GUI::createWindow(Vector4 D,String M, wt T,String C){Window* w=new Window(D,M,T,C,this);WN.push_back(w);return w;}
Window::Window(Vector4 D,String M, wt t,String C, GUI *G):x(D.x),y(D.y),w(D.z),h(D.w),mGUI(G),mTB(0),mRZ(0),mATI(0),mAB(0){
	mO=G->createOverlay("BetaGUI.w"+StringConverter::toString(G->wc++),Vector2(D.x,D.y),Vector2(D.z,D.w),M); 
	if(t>=2){Callback c;c.t=4;mRZ=createButton(Vector4(D.z-16,D.w-16,16,16),M+".resize","",c);}
	if(t==1||t==3){Callback c;c.t=3;mTB=createButton(Vector4(0,0,D.z-22,22),M+".titlebar",C,c);}}
Window::~Window(){for(uint i=0;i<mB.size();i++)delete mB[i];for(uint i=0;i<mT.size();i++)delete mT[i];mGUI->mO->remove2D(mO);}
Button* Window::createButton(Vector4 D,String M,String T,Callback C) {Button *x=new Button(D,M,T,C,this);mB.push_back(x);return x;}
Button::Button(Vector4 D, String M, String T, Callback C, Window *P):x(D.x),y(D.y),w(D.z),h(D.w),mmn(M),mma(M){
	ResourcePtr ma=MaterialManager::getSingleton().getByName(mmn+".active");if(!ma.isNull())mma+=".active";
	mO=P->mGUI->createOverlay(P->mO->getName()+"b"+StringConverter::toString(P->mGUI->bc++),Vector2(x,y),Vector2(w,h),M,"",false);
	mCP=P->mGUI->createOverlay(mO->getName()+"c",Vector2(4,(h-P->mGUI->mFontSize)/2),Vector2(w,h),"",T,false);
	P->mO->addChild(mO);mO->show();mO->addChild(mCP);mCP->show();callback=C;}
TextInput* Window::createTextInput(Vector4 D,String M,String V,uint L){TextInput *x=new TextInput(D,M,V,L,this);mT.push_back(x);return x;}
OverlayContainer* Window::createStaticText(Vector4 D,String T){OverlayContainer* x=mGUI->createOverlay(mO->getName()+StringConverter::toString(mGUI->tc++),
	Vector2(D.x,D.y),Vector2(D.z,D.w),"", T,false);mO->addChild(x);x->show();return x;}
TextInput::TextInput(Vector4 D,String M,String V,uint L,Window *P):x(D.x),y(D.y),w(D.z),h(D.w),value(V),mmn(M),mma(M),length(L) {
	ResourcePtr ma=Ogre::MaterialManager::getSingleton().getByName(mmn+".active");if(!ma.isNull())mma+=".active";
	mO=P->mGUI->createOverlay(P->mO->getName()+"t"+StringConverter::toString(P->mGUI->tc++) ,Vector2(x,y),Vector2(w,h),M,"",false);
	mCP=P->mGUI->createOverlay(mO->getName()+"c",Vector2(4,(h-P->mGUI->mFontSize)/2),Vector2(w,h),"",V,false);
	P->mO->addChild(mO);mO->show();mO->addChild(mCP);mCP->show();}
bool Window::checkKey(String k, uint px, uint py){if(mATI==0)return false;if(!mO->isVisible())return false;
	if(!(px>=x&&py>=y)||!(px<=x+w&&py<=y+h))return false;if(k=="!b"){mATI->setValue(mATI->value.substr(0,mATI->value.length()-1));
	return true;}if(mATI->value.length() >= mATI->length)return true;mATI->mCP->setCaption(mATI->value+=k);return true;}
bool Window::check(uint px, uint py, bool LMB){if(!mO->isVisible())return false;
	if(!(px>=x&&py>=y)||!(px<=x+w&&py<=y+h)){if(mAB){mAB->activate(false);}return false;}
	if(mAB){mAB->activate(false);}for(uint i=0;i<mB.size();i++){if (mB[i]->in(px,py,x,y))continue;
	if(mAB){mAB->activate(false);}mAB=mB[i];mAB->activate(true);if(!LMB)return true;
	if(mATI){mATI->mO->setMaterialName(mATI->mmn);mATI=0;}switch(mAB->callback.t){default: return true;
	case 1:mAB->callback.fp(mAB);return true;case 2:mAB->callback.LS->onButtonPress(mAB);return true;
	case 3:mO->setPosition(x=px-(mAB->w/2),y=py-(mAB->h/2));return true;case 4:mO->setDimensions(w=px-x+8,h=py-y+8);
	mRZ->mO->setPosition(mRZ->x=w-16,mRZ->y=h-16);if(mTB){mTB->mO->setWidth(mTB->w=w);}return true;}}if (!LMB)return true;
	for(uint i=0;i<mT.size();i++){if(mT[i]->in(px,py,x,y))continue;mATI=mT[i];mATI->mO->setMaterialName(mATI->mma);return true;}
	if(mATI){mATI->mO->setMaterialName(mATI->mmn);mATI=0;}return true;}
} // End of Betajaen's GUI. Normal programming can resume now.

//***************************************************************************************************
//***************************************************************************************************


//***************************************************************************************************
//***************************************************************************************************

#ifdef USE_BETASOUND
/// Betajaen's Sound 001
/// Written by Robin "Betajaen" Southern 27-Nov-2006, http://www.ogre3d.org/wiki/index.php/BetaSound
/// This code is under the Whatevar! licence. Do what you want; but keep the original copyright header.
/// This code is not meant to be readable, if you base your future source on this, I will laugh at you.
#pragma comment(lib, "dxguid.lib")
#define INITGUID
#include "Ogre.h"
#include <windows.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <cguid.h>
namespace BetaSound {
	class Concert; class Speaker;
	enum PLAY_TYPE {ONCE, ONCE_AND_DELETE, LOOP};
	class Concert {friend Speaker;
	public:		Concert(SceneManager *mgr);~Concert();
				Speaker* createSpeaker(std::string mFilename, Ogre::SceneNode *mNode, BetaSound::PLAY_TYPE type);
				void destroySpeaker(Speaker *s);void destroyAllSpeakers();
				void update(Vector3 position);
	protected:	IDirectMusicPerformance8* mPerformance;		IDirectMusicLoader8* mLoader;
				Ogre::SceneManager *mSceneMgr;				std::map<Ogre::String, Speaker*> mSpeakers;
				std::map<Ogre::String, IDirectMusicAudioPath8*> mAudioPath; 
				Ogre::Vector3 mEar;}; 

				class Speaker {friend Concert;
				protected:	Speaker(Ogre::String mFilename,Ogre::SceneNode *mNode,PLAY_TYPE type,Concert *concert);~Speaker();
							IDirectMusicSegment8* mSegment;				IDirectMusicAudioPath8* m3DAudioPath;
							IDirectSound3DBuffer8* mDSB;				IDirectSound3DListener8* mListener;
							BetaSound::PLAY_TYPE mType;					bool mDeleteMe;
				public:		void setVolume(unsigned int v) {/*m3DAudioPath->SetVolume(v); */}
							void stop() {mConcert->mPerformance->Stop(mSegment, 0, 0,0);}
							void play() {mConcert->mPerformance->PlaySegmentEx(mSegment,NULL,NULL,DMUS_SEGF_SECONDARY,0,NULL,NULL,m3DAudioPath);}
							void refresh(){m3DAudioPath->Activate(true);play();}
							void update(Vector3 position);
							Concert	   *mConcert;SceneNode  *mNode;};
};/// End of headers
/// >>>>>>>>>>>> Optional .CPP part: Cut and paste downwards and uncomment the next line. <<<<<<<<<<<<<<<
//#include "BetaSound.h"
namespace BetaSound {
	Concert::Concert(SceneManager *mgr) : mSceneMgr(mgr) {
		CoInitialize(NULL);
		CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC,IID_IDirectMusicLoader8, (void**)&mLoader);
		CoCreateInstance(CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC,IID_IDirectMusicPerformance8,(void**)&mPerformance); 
		mPerformance->InitAudio(NULL, NULL, NULL, DMUS_APATH_DYNAMIC_3D, 64, DMUS_AUDIOF_ALL, NULL);}
	Concert::~Concert() {
		for(std::map<Ogre::String, Speaker* >::iterator i = mSpeakers.begin();i != mSpeakers.end();++i)
			delete (*i).second;
		if (mPerformance)		mPerformance->Stop(NULL,NULL,0,0);
		if (mLoader){			mLoader->Release();mLoader=NULL;}
		if (mPerformance){		mPerformance->CloseDown();mPerformance->Release();mPerformance=NULL;}
		CoUninitialize();}
	void Concert::update(Ogre::Vector3 position) {
		for(std::map< Ogre::String, Speaker* >::iterator i = mSpeakers.begin();i != mSpeakers.end();++i) {
			if ((*i).second->mDeleteMe){
				delete (*i).second;mSpeakers.erase(i);
				if (mSpeakers.size() >=1)	i = mSpeakers.begin();	else	break;}
			(*i).second->update(position);}
		mEar = position;}
	Speaker* Concert::createSpeaker(Ogre::String mFilename, Ogre::SceneNode *mNode, BetaSound::PLAY_TYPE type) {
		if (mSpeakers[mNode->getName() + "." + mFilename]) return 0;
		mSpeakers[mNode->getName() + "." + mFilename] = new Speaker(mFilename, mNode, type, this);
		return mSpeakers[mNode->getName() + "." + mFilename];}
	void Concert::destroySpeaker(Speaker *s) {	s->stop();
	for(std::map<Ogre::String,Speaker*>::iterator i=mSpeakers.begin();i!=mSpeakers.end();++i){delete(*i).second;mSpeakers.erase(i);return;}}
	Speaker::Speaker(std::string mFilename,Ogre::SceneNode *node,PLAY_TYPE type,Concert *concert):mConcert(concert),mNode(node),mType(type){
		WCHAR wF[MAX_PATH];MultiByteToWideChar(CP_ACP,0,(char*)mFilename.c_str(),-1,wF,MAX_PATH);
		mConcert->mLoader->LoadObjectFromFile(CLSID_DirectMusicSegment,IID_IDirectMusicSegment8,wF,(LPVOID*)&mSegment);
		mSegment->Download(mConcert->mPerformance);
		if(mType==LOOP)mSegment->SetRepeats(DMUS_SEG_REPEAT_INFINITE);else mSegment->SetRepeats(0);
		if (!mConcert->mAudioPath[mFilename])
			mConcert->mPerformance->CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D,64,TRUE,&concert->mAudioPath[mFilename]);
		mConcert->mAudioPath[mFilename]->GetObjectInPath(DMUS_PCHANNEL_ALL,DMUS_PATH_BUFFER,0,GUID_NULL,0,IID_IDirectSound3DBuffer,(LPVOID*) &mDSB);
		mConcert->mAudioPath[mFilename]->GetObjectInPath(0,DMUS_PATH_PRIMARY_BUFFER,0,GUID_All_Objects,0,IID_IDirectSound3DListener,(void **)&mListener);
		update(mConcert->mEar);play();}
	Speaker::~Speaker() {
		mDSB->Release();mDSB=NULL;						mListener->Release();mListener=NULL;
		/*m3DAudioPath->Release();m3DAudioPath=NULL;*/	mSegment->Release();mSegment = NULL;}
	void Speaker::update(Ogre::Vector3 position) {
		if (mType == BetaSound::ONCE_AND_DELETE)	mDeleteMe = true;
		mDSB->SetPosition(mNode->getPosition().x,mNode->getPosition().y,mNode->getPosition().z,DS3D_IMMEDIATE);
		mListener->SetPosition(position.x,position.y,position.y,DS3D_IMMEDIATE);}
}// End of BetaSound
#endif

//***************************************************************************************************
//***************************************************************************************************

#endif