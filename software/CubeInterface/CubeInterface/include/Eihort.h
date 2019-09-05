///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Tutorial Template for OGRE 1.4.3 (EIHORT) with modifications for use with NxOGRE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EIHORT_H
#define EIHORT_H

#define NOMINMAX
#include "windows.h"
#include "resource.h"

#include "nxOgre.h"
#include "Ogre.h"
#include "OgreConfigFile.h"

#include "OgreStringConverter.h"
#include "OgreException.h"

using namespace Ogre;
using namespace nxOgre;
using namespace std;

#ifdef _DEBUG
#pragma comment(lib, "OIS_d.lib")
#else
#pragma comment(lib, "OIS.lib")
#endif
#define OIS_DYNAMIC_LIB
#include "OIS/OIS.h"
using namespace OIS;

#include "BetaGUI.h"

class Eihort : public FrameListener, public OIS::KeyListener, public BetaGUI::BetaGUIListener {

	public:

		// Enums
		enum Key {
			QUIT,SCREENSHOT,DEBUG_MODE,RESET_SCENE, PAUSE, SLOWER, FASTER,
			OPTION_1,OPTION_2,OPTION_3,OPTION_4,
			PAN_FORWARD,PAN_BACKWARD,PAN_LEFT,PAN_RIGHT,PAN_UP,PAN_DOWN,
			MOUSE_PRIMARY,MOUSE_ALT,MOUSE_NONE,MOUSE_BOTH,MOUSE_MIDDLE,
			Q,W,E,R,T,Y,U,I,O,P,A,S,D,F,G,H,J,K,L,Z,X,C,V,B,N,M
		};

		enum mouseMode {
			CAMERA_FOLLOW,CAMERA_FORCE,CAMERA_CONTROL,PICK
		};

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		Eihort() {
			mTutorialName = "Unnamed";
			mTutorialDescription = "No Description";
			mShutdownNow = false;
			mMouseMode = PICK;
			mConstruct = false;

			// Control bits
			cube_haltRotation = false;
			cube_rotateClockwise = false;
			cube_rotateCounterClockwise = false;
			recordStart=false;
			record=false;
			recordStop=false;
			cube_add_parameter=false;
			cube_edit=true;
			cube_typeCube=false;
			cube_add=false;
			cube_addCubePosY = false;
			cube_addCubeNegY = false;
			cube_addCubePosZ = false;
			cube_addCubeNegZ = false;
			cube_addCubePosX = false;
			cube_addCubeNegX = false;
			cube_deleteCube = false;

			cube_restart = false;
			cube_evolve = false;
			cube_playEvolve = false;
			cube_getData = false;

			cube_orientationType = false;
			cube_orientation = 2;
			cube_faceOnTop=false;
			faceOnTop=3;
			cube_angle = 0;
			cube_angleDisplay = 0;
			cube_typeCubeID = 1;
			cube_typeCubeClass = 0xF7;
			
			cube_rotation=false;
			cube_rotation_begin=true;
			cube_rotation_end=false;

			height_star=4;
			saveCube = false;
			loadCube = false;
			cube_Face1Inc=false;
		    cube_Face2Inc=false;
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		HINSTANCE hInst;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void setHInst(HINSTANCE hi) {
			hInst = hi;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		~Eihort() {
			std::cout << "Exiting" << std::endl;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool Run() {
				getSetup();

				resetConfig();
				getConfig();

				setupRenderSystem();
			
				ConfigFile cf;
				cf.load("media/resources.cfg");

				ConfigFile::SectionIterator seci = cf.getSectionIterator();
				String secName, typeName, archName;
				
				while (seci.hasMoreElements()) {
					secName = seci.peekNextKey();
					ConfigFile::SettingsMultiMap *settings = seci.getNext();
					ConfigFile::SettingsMultiMap::iterator i;
					for (i = settings->begin(); i != settings->end(); ++i) {
						typeName = i->first;
						archName = i->second;
						ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
					}
				}
				
				mWindow = mRoot->initialise(true, mTutorialName + " - " + mTutorialDescription);

				mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "NxOgre");
				mCamera = mSceneMgr->createCamera("MainCamera");
				mCamera->setPosition(Vector3(0,0,0));
				mCamera->lookAt(Vector3(0,0,0));
				mCamera->setNearClipDistance(0.1);

				mViewport = mWindow->addViewport(mCamera);

				if (!mConstruct)
					mViewport->setBackgroundColour(ColourValue::Black);
				else
					mViewport->setBackgroundColour(ColourValue::White);
				mCamera->setAspectRatio(Real(mViewport->getActualWidth()) / Real(mViewport->getActualHeight()));


				TextureManager::getSingleton().setDefaultNumMipmaps(1);
				MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
				MaterialManager::getSingleton().setDefaultAnisotropy(8); 
				ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

				mOverlay = Ogre::OverlayManager::getSingleton().create("GUI");
				mOverlay->setZOrder(549);

				Ogre::String loadingBackMat = "boot.any";

				if (!MaterialManager::getSingleton().getByName("boot." + mTutorialName).isNull())
					loadingBackMat = "boot." + mTutorialName;
				
				OverlayElement* loadingBack = createOverlay("loadBack",Vector2(0,0),Vector2(mViewport->getActualWidth(),mViewport->getActualHeight()),loadingBackMat);		
				OverlayElement* loading = createOverlay("load",Vector2((mWindow->getWidth() / 2) - 64,(mWindow->getHeight() / 2) - 16),Vector2(128,32),"gui_loading");

				mOverlay->show();

				mRoot->renderOneFrame();
				
				loading->hide();
				loadingBack->hide();
				
				createInputSys(mWindow);
								
				mGUI = new BetaGUI::GUI("nxogrefont",16);
				mGUI_ExitWindow = mGUI->createWindow(Vector4(mWindow->getWidth() - 128- 4,4,128,30), "", BetaGUI::NONE);
				mGUI_ExitButton =  mGUI_ExitWindow->createButton(Vector4(128 -  30,0,30,30), "bgui.exit", "", BetaGUI::Callback(this));
				mGUI_PauseButton = mGUI_ExitWindow->createButton(Vector4(128 - (30 * 2) - 4,0,30,30), "bgui.pause", "", BetaGUI::Callback(this));
				mGUI_DebugButton = mGUI_ExitWindow->createButton(Vector4(128 - (30 * 3) - 4,0,30,30), "bgui.debug", "", BetaGUI::Callback(this));

				//*******************************************************************				
			
				// GUI element positions are manually set

				// Control window
				int position=0;
				mGUI_ControlWindow = mGUI->createWindow(Vector4(10,10,160,644), "bgui.window", BetaGUI::NONE, "CONTROL");
				
				
				position++;
				mGUI_ControlWindow->createStaticText(Vector4(17,25*position++,198,40)," INITIALIZATION");
				mGUI_CubeFaceOnTop = mGUI_ControlWindow->createTextInput(Vector4(10, (25 *position),104,25), "bgui.textinput", "FACE ON TOP",0);
				mGUI_CubeFaceOnTopDec = mGUI_ControlWindow->createButton(Vector4(114,(25 *position),18,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_CubeFaceOnTopInc = mGUI_ControlWindow->createButton(Vector4(132, (25 *position++),18,25), "bgui.button", ">", BetaGUI::Callback(this));
				mGUI_CubeHight = mGUI_ControlWindow->createTextInput(Vector4(10, (25 *position),60,25), "bgui.textinput", "HEIGHT",0);
				mGUI_CubeHightDec10 = mGUI_ControlWindow->createButton(Vector4(70,(25 * position),20,25), "bgui.button", "<<", BetaGUI::Callback(this));
				mGUI_CubeHightDec = mGUI_ControlWindow->createButton(Vector4(90,(25 * position),20,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_CubeHightInc = mGUI_ControlWindow->createButton(Vector4(110,(25 *position),20,25), "bgui.button", ">", BetaGUI::Callback(this));			
				mGUI_CubeHightInc10 = mGUI_ControlWindow->createButton(Vector4(130, (25 *position++),20,25), "bgui.button", ">>", BetaGUI::Callback(this));	
				mGUI_restart = mGUI_ControlWindow->createButton(Vector4(10,  (25 *position++),140,25), "bgui.button", "RESTART", BetaGUI::Callback(this));
				
				position++;
				mGUI_ControlWindow->createStaticText(Vector4(15,(25 * position++),198,40), "  MORPHOLOGY");
				
				mGUI_edit = mGUI_ControlWindow->createButton(Vector4(10,(25 * position),100,25), "bgui.button", "EDITION", BetaGUI::Callback(this));
				mGUI_editState = mGUI_ControlWindow->createTextInput(Vector4(110,(25 *position++),40,25), "bgui.textinput", "ON",0);

				mGUI_typeCube = mGUI_ControlWindow->createTextInput(Vector4(10,(25 * position),100,25), "bgui.textinput", "CUBE",0);
				mGUI_typeCubeDec = mGUI_ControlWindow->createButton(Vector4(110,(25 * position),20,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_typeCubeInc = mGUI_ControlWindow->createButton(Vector4(130,(25 * position++),20,25), "bgui.button", ">", BetaGUI::Callback(this));
					
			    mGUI_CubeOrientation = mGUI_ControlWindow->createTextInput(Vector4(10,(25 * position),100,25), "bgui.textinput", "ROTATION",0);
				mGUI_CubeOrientationDec = mGUI_ControlWindow->createButton(Vector4(110,(25 * position),20,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_CubeOrientationInc = mGUI_ControlWindow->createButton(Vector4(130, (25 * position++),20,25), "bgui.button", ">", BetaGUI::Callback(this));

				mGUI_angle = mGUI_ControlWindow->createTextInput(Vector4(10, (25 * position),60,25), "bgui.textinput", "ANGLE",0);
				mGUI_angleDec10 = mGUI_ControlWindow->createButton(Vector4(70, (25 * position),20,25), "bgui.button", "<<", BetaGUI::Callback(this));
				mGUI_angleDec = mGUI_ControlWindow->createButton(Vector4(90,(25 * position),20,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_angleInc = mGUI_ControlWindow->createButton(Vector4(110,(25 * position),20,25), "bgui.button", ">", BetaGUI::Callback(this));			
				mGUI_angleInc10 = mGUI_ControlWindow->createButton(Vector4(130,(25 * position++),20,25), "bgui.button", ">>", BetaGUI::Callback(this));			

				mGUI_CubeFace1 = mGUI_ControlWindow->createTextInput(Vector4(10,(25 * position),100,25), "bgui.textinput", "POSITION",0);
				mGUI_CubeFace1Dec = mGUI_ControlWindow->createButton(Vector4(110,(25 * position),20,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_CubeFace1Inc = mGUI_ControlWindow->createButton(Vector4(130,(25 * position++),20,25), "bgui.button", ">", BetaGUI::Callback(this));

				mGUI_CubeFace2 = mGUI_ControlWindow->createTextInput(Vector4(10,(25 * position),100,25), "bgui.textinput", "ORIENTATION",0);
				mGUI_CubeFace2Dec = mGUI_ControlWindow->createButton(Vector4(110,(25 * position),20,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_CubeFace2Inc = mGUI_ControlWindow->createButton(Vector4(130,(25 * position++),20,25), "bgui.button", ">", BetaGUI::Callback(this));
				
				mGUI_addCube=mGUI_ControlWindow->createButton(Vector4(10,(25 * position++),140,25), "bgui.button", "ADD CUBE", BetaGUI::Callback(this));				

				mGUI_deleteCube = mGUI_ControlWindow->createButton(Vector4(10,(25 * position++),140,25), "bgui.button", "DELETE CUBE", BetaGUI::Callback(this));

				position++;
				mGUI_ControlWindow->createStaticText(Vector4(20,(25 * position++),198,40), "   CONTROL");

				mGUI_setAngle = mGUI_ControlWindow->createTextInput(Vector4(10, (25 * position),60,25), "bgui.textinput", "ANGLE",0);
				mGUI_setAngleDec10 = mGUI_ControlWindow->createButton(Vector4(70, (25 * position),20,25), "bgui.button", "<<", BetaGUI::Callback(this));
				mGUI_setAngleDec = mGUI_ControlWindow->createButton(Vector4(90,(25 * position),20,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_setAngleInc = mGUI_ControlWindow->createButton(Vector4(110,(25 * position),20,25), "bgui.button", ">", BetaGUI::Callback(this));			
				mGUI_setAngleInc10 = mGUI_ControlWindow->createButton(Vector4(130,(25 * position++),20,25), "bgui.button", ">>", BetaGUI::Callback(this));	

				mGUI_rotate=mGUI_ControlWindow->createTextInput(Vector4(10,(25 * position),80,25), "bgui.textinput", "ROTATION",0);
				mGUI_rotateClockwise = mGUI_ControlWindow->createButton(Vector4(90,(25 * position),20,25), "bgui.button", "<", BetaGUI::Callback(this));
				mGUI_rotateCounterClockwise = mGUI_ControlWindow->createButton(Vector4(130,(25 * position),20,25), "bgui.button", ">", BetaGUI::Callback(this));				
				mGUI_haltRotation = mGUI_ControlWindow->createButton(Vector4(110,(25 * position++),20,25), "bgui.button", "| |", BetaGUI::Callback(this));
				mGUI_record = mGUI_ControlWindow->createButton(Vector4(10,(25 * position),100,25), "bgui.button", "RECORD", BetaGUI::Callback(this));
				mGUI_recordState = mGUI_ControlWindow->createTextInput(Vector4(110,(25 * position++),40,25), "bgui.textinput", "STOP",0);

				position++;
				mGUI_ControlWindow->createStaticText(Vector4(25,(25 * position++),198,40), "  EVOLUTION");

				mGUI_evolve = mGUI_ControlWindow->createButton(Vector4(10,(25 * position),100,25), "bgui.button", "EVOLVE", BetaGUI::Callback(this));
				mGUI_evolveState = mGUI_ControlWindow->createTextInput(Vector4(110,(25 * position++),40,25), "bgui.textinput", "OFF",0);

				mGUI_playEvolve = mGUI_ControlWindow->createButton(Vector4(10, (25 * position),100,25), "bgui.button", "PLAY RESULT", BetaGUI::Callback(this));
				mGUI_playEvolveState = mGUI_ControlWindow->createTextInput(Vector4(110,(25 * position++),40,25), "bgui.textinput", "OFF",0);

				mGUI_getData = mGUI_ControlWindow->createButton(Vector4(10,(25 * position),100,25), "bgui.button", "GET DATA", BetaGUI::Callback(this));
				mGUI_getDataState = mGUI_ControlWindow->createTextInput(Vector4(110,(25 * position++),40,25), "bgui.textinput", "OFF",0);

				// Instruction window
				mGUI_InstructionWindow = mGUI->createWindow(Vector4(170,10,335,90), "bgui.window", BetaGUI::NONE, "INSTRUCTIONS");
				mGUI_InstructionWindow->createStaticText(Vector4(8,22,198,40),
					"Right-click to select cubes\nHold both mouse buttons down to look around\nUse WASD to move the camera\nLoading must be done at the beginning");
				
				// Save/load window
				mGUI_SaveLoadWindow = mGUI->createWindow(Vector4(510,10,160,90), "bgui.window", BetaGUI::NONE, "SAVELOAD");
				mGUI_saveCube = mGUI_SaveLoadWindow->createButton(Vector4(10,50,58,32), "bgui.button", " SAVE", BetaGUI::Callback(this));
				mGUI_loadCube = mGUI_SaveLoadWindow->createButton(Vector4(92,50,58,32), "bgui.button", " LOAD", BetaGUI::Callback(this));
				mGUI_fileName = mGUI_SaveLoadWindow->createTextInput(Vector4(10,10,140,32), "bgui.textinput", "star.mmf", 18);
			
				
				// Evolution window
				position=1;
				mGUI_EvolutionWindow = mGUI->createWindow(Vector4(mWindow->getWidth()-40-320,mWindow->getHeight()-35-200,320,200), "bgui.window", BetaGUI::MOVE, "EVOLUTION");
				mGUI_EvolutionExitButton =  mGUI_EvolutionWindow->createButton(Vector4(320-19,4,15,15), "bgui.exit", "", BetaGUI::Callback(this));
				mGUI_EvolutionWindow->hide();

				
				mGUI_EvolutionWindow->createStaticText(Vector4(25,(25 * position)+12,150,25), "Generation:");
				mGUI_EvolutionGeneration = mGUI_EvolutionWindow->createTextInput(Vector4(160,(25 * position++)+12,85,20), "bgui.textinput", "0", 0);						
				mGUI_EvolutionWindow->createStaticText(Vector4(25,(25 * position)+12,150,25), "Current individu:");
				mGUI_EvolutionIndividu = mGUI_EvolutionWindow->createTextInput(Vector4(160,(25 * position++)+12,85,20), "bgui.textinput", "0", 0);	
				mGUI_EvolutionWindow->createStaticText(Vector4(25,(25 * position)+12,150,25), "Time experience:");
				mGUI_EvolutionTime = mGUI_EvolutionWindow->createTextInput(Vector4(160,(25 * position++)+12,85,20), "bgui.textinput", "0", 0);
				mGUI_EvolutionWindow->createStaticText(Vector4(25,(25 * position)+12,150,25), "Best individu:");
				mGUI_EvolutionBest = mGUI_EvolutionWindow->createTextInput(Vector4(160,(25 * position++)+12,85,20), "bgui.textinput", "0", 0);
				mGUI_EvolutionWindow->createStaticText(Vector4(25,(25 * position)+12,150,40), "Average:");
				mGUI_EvolutionAverage = mGUI_EvolutionWindow->createTextInput(Vector4(160,(25 * position++)+12,85,20), "bgui.textinput", "0", 0);
				mGUI_EvolutionWindow->createStaticText(Vector4(25,(25 * position)+12,150,25), "Size population:");
				mGUI_EvolutionPopulation = mGUI_EvolutionWindow->createTextInput(Vector4(160,(25 * position++)+12,85,20), "bgui.textinput", "0", 0);
				
				//*******************************************************************

				//mPointer = createOverlay("Pointer", Vector2((mWindow->getWidth() / 2) - 16, (mWindow->getHeight() / 2) - 16), Vector2(32,32), "nx.arrow");
				//mPointer->hide();		
				mPointer = mGUI->createMousePointer(Vector2(24,24), "bgui.pointer");
				mGUI->injectMouse((mWindow->getWidth() / 2) - 16, (mWindow->getHeight() / 2) - 16, false);
				mPointer->hide();

				gui_pause = createOverlay("paused", Vector2(16, mWindow->getHeight() - 100 - 32), Vector2(128,128), "gui_pause");
				gui_pause->hide();

				gui_slowfast = createOverlay("slowfast", Vector2(16 + 100, mWindow->getHeight() - 100 - 32), Vector2(128,128), "", "0");
				gui_slowfast->hide();
				
				mCamera->lookAt(0,0,0);
				
				mCaption1 = createOverlay("caption.1",Vector2(8,128),Vector2(16,256),""," ");
				mCaption2 = createOverlay("caption.2",Vector2(8,128 + 16),Vector2(16,256),""," ");
				mCaption3 = createOverlay("caption.3",Vector2(8,mWindow->getHeight() - 16 - 8),Vector2(16,256),""," ");
				
				ApplicationStart();
				
				if (!mConstruct) {
					sg = mSceneMgr->createStaticGeometry("grid");
						CreateEntNode("nx.floor2", Vector3(0,-0.05,0));
						CreateEntNode("nx.body.axis", Vector3(0,0,0));
					sg->build();
				}

				if (!mConstruct) {
					mSceneMgr->setFog(FOG_LINEAR, ColourValue::Black, 0.0005f, 4000,4500);
					mSceneMgr->setAmbientLight(ColourValue(0.6,0.6,0.6));
				}
				else {
					mSceneMgr->setFog(FOG_LINEAR, ColourValue::White, 0.0005f, 4000,4500);
					mSceneMgr->setAmbientLight(ColourValue(0.9,0.9,0.9));
				}
		       
				mSceneMgr->setShadowTechnique( SHADOWTYPE_STENCIL_ADDITIVE );

				/*
					mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);s
					mSceneMgr->setShadowFarDistance(100);
					mSceneMgr->setShowDebugShadows(true);
					mSceneMgr->setShadowTexturePixelFormat(PF_X8R8G8B8);
					mSceneMgr->setShadowTextureSettings(1024, 2);
				*/
					
				Light* mLight =  mSceneMgr->createLight("SunLight");
				mLight->setPosition( Vector3(-150, 100, 50) );
				

				if (mMouseMode == PICK)
					mPointer->show();

				mRoot->addFrameListener(this);				
				mRoot->startRendering();
				return true;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void setupRenderSystem() {
			mRoot = new Root("","","ogre.graphics.log");
			mRoot->loadPlugin("RenderSystem_Direct3D9");
			mRoot->loadPlugin("RenderSystem_GL");
			mRoot->loadPlugin("Plugin_ParticleFX");
			
			Ogre::String rs = "Direct3D9 Rendering Subsystem";

			Ogre::RenderSystemList *pRenderSystemList; 
			pRenderSystemList = mRoot->getAvailableRenderers(); 
			Ogre::RenderSystemList::iterator pRenderSystem; 
			pRenderSystem = pRenderSystemList->begin(); 
			Ogre::RenderSystem *pSelectedRenderSystem; 
			pSelectedRenderSystem = *pRenderSystem; 
		
			while (pRenderSystem != pRenderSystemList->end())  {
				
				if ((*pRenderSystem)->getName() == rs) {
					mRenderSystem = *pRenderSystem;
					break;
				}
				
				pRenderSystem++;
			}

			mRoot->setRenderSystem(mRenderSystem);

			mRenderSystem->setConfigOption("Full Screen", settings["fullscreen"]);
			mRenderSystem->setConfigOption("VSync", settings["vsync"]);

			if (rs == "Direct3D9 Rendering Subsystem") {
				mRenderSystem->setConfigOption("Video Mode", 
									   settings["width"]
									   + " x " +  settings["height"]
									   + " @ " +  settings["depth"] + "-bit colour"  
				);
				
				//mRenderSystem->setConfigOption("Anti aliasing","Level 4");

			}
			else {
				mRenderSystem->setConfigOption("Video Mode", settings["width"]   + " x " + settings["height"]
				);

				mRenderSystem->setConfigOption("Colour Depth",settings["depth"]);
				//mRenderSystem->setConfigOption("FSAA", "2");
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void resetConfig() {

			settings.clear();
			settings["device"] = "Direct3D9 Rendering Subsystem";
			settings["width"] = "800";
			settings["height"] = "600";
			settings["depth"] = "32";
			settings["aa"] = "0";
			settings["vsync"] = "No";
			settings["fullscreen"] = "No";

			keys.clear();
			keys[QUIT] = KC_ESCAPE;
			keys[SCREENSHOT] = KC_F1;
			keys[DEBUG_MODE] = KC_F2;
			keys[PAUSE] = KC_F3;
			keys[SLOWER] = KC_F4;
			keys[FASTER] = KC_F5;
			keys[RESET_SCENE] = KC_F12;
			keys[OPTION_1] = KC_1;
			keys[OPTION_2] = KC_2;
			keys[OPTION_3] = KC_3;
			keys[OPTION_4] = KC_4;
			keys[PAN_FORWARD] = KC_W;
			keys[PAN_BACKWARD] = KC_S;
			keys[PAN_LEFT] = KC_A;
			keys[PAN_RIGHT] = KC_D;
			keys[PAN_UP] = KC_Q;
			keys[PAN_DOWN] = KC_Z;

			// All 26 letters in the alphabet
			keys[Q] = KC_Q;
			keys[W] = KC_W;
			keys[E] = KC_E;
			keys[R] = KC_R;
			keys[T] = KC_T;
			keys[Y] = KC_Y;
			keys[U] = KC_U;
			keys[I] = KC_I;
			keys[O] = KC_O;
			keys[P] = KC_P;
			keys[A] = KC_A;
			keys[S] = KC_S;
			keys[D] = KC_D;
			keys[F] = KC_F;
			keys[G] = KC_G;
			keys[H] = KC_H;
			keys[J] = KC_J;
			keys[K] = KC_K;
			keys[L] = KC_L;
			keys[Z] = KC_Z;
			keys[X] = KC_X;
			keys[C] = KC_C;
			keys[V] = KC_V;
			keys[B] = KC_B;
			keys[N] = KC_N;
			keys[M] = KC_M;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void getConfig() {

			ConfigFile cf;
			cf.load("config.yaml",":",true);

			ConfigFile::SectionIterator seci = cf.getSectionIterator();
			String secName, typeName, archName;
			
			while (seci.hasMoreElements()) {
				secName = seci.peekNextKey();
				ConfigFile::SettingsMultiMap *csettings = seci.getNext();
				ConfigFile::SettingsMultiMap::iterator i;

				for (i = csettings->begin(); i != csettings->end(); ++i) {
					
					if (i->first == "device") {
						if (i->second == "dx9") {
							settings["device"] = "Direct3D9 Rendering Subsystem";
						}
						else {
							settings["device"] = "OpenGL Rendering Subsystem"; // Check..
						}
					}
					else if (i->first == "width") {
						settings["width"] = Ogre::StringConverter::toString(Ogre::StringConverter::parseUnsignedInt(i->second));
					}
					else if (i->first == "height") {
						settings["height"] = Ogre::StringConverter::toString(Ogre::StringConverter::parseUnsignedInt(i->second));
					}
					else if (i->first == "depth") {
						settings["depth"] = Ogre::StringConverter::toString(Ogre::StringConverter::parseUnsignedInt(i->second));
					}
					else if (i->first == "aa") {
						settings["aa"] = Ogre::StringConverter::toString(Ogre::StringConverter::parseUnsignedInt(i->second));
					}
					else if (i->first == "fullscreen") {
						settings["fullscreen"] = i->second;
					}
					else if (i->first == "vsync") {
						settings["aa"] = i->second;
					}

				}
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void createInputSys(RenderWindow *_window) {
			LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
			ParamList pl;	
			size_t windowHnd = 0;
			std::ostringstream windowHndStr;

			_window->getCustomAttribute("WINDOW", &windowHnd);
			windowHndStr << windowHnd;
			pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

			/*
			#if defined OIS_WIN32_PLATFORM
			pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
			pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
			pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
			pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
			#elif defined OIS_LINUX_PLATFORM
			pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
			pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
			pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
			pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
			#endif
			*/

			InputManager &im = *OIS::InputManager::createInputSystem( pl );

			mKeyboard = static_cast<Keyboard*>(im.createInputObject( OISKeyboard, true ));
			mMouse = static_cast<Mouse*>(im.createInputObject( OISMouse, true ));
			mKeyboard->setEventCallback(this);
			mKeyboard->setBuffered(true);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void onButtonPress(BetaGUI::Button *ref) {

			if(ref == mGUI_saveCube && timeSince >= 0.2f) {
				saveCube = true;
				timeSince = 0.0f;
			}
			
			if(ref == mGUI_loadCube && timeSince >= 0.2f) {
				loadCube = true;
				mGUI_playEvolveState->setValue("OFF");
				mGUI_evolveState->setValue("OFF");
                mGUI_getDataState->setValue("OFF");
				timeSince = 0.0f;					
			}
				
			if(ref == mGUI_edit && timeSince >= 0.2f){
				cube_edit=1-cube_edit;
				if(cube_edit){
				 cube_add_parameter=true;
				 mGUI_editState->setValue("ON");
				}
				else
				 mGUI_editState->setValue("OFF");
				 timeSince = 0.0f;
			}

			if(ref == mGUI_typeCubeDec && timeSince >= 0.2f) {
				cube_typeCubeID--;
				cube_add_parameter=true;
				cube_typeCube=true;					
				timeSince = 0.0f;
			}

			if(ref == mGUI_typeCubeInc && timeSince >= 0.2f) {
				cube_typeCubeID++;
				cube_add_parameter=true;
				cube_typeCube=true;	
				timeSince = 0.0f;
			}

			if(ref == mGUI_CubeOrientationDec && timeSince >= 0.2f) {
				cube_orientation--;
				cube_add_parameter=true;
				cube_orientationType=true;					
				timeSince = 0.0f;
			}

			if(ref == mGUI_CubeOrientationInc && timeSince >= 0.2f) {
				cube_orientation++;
				cube_orientationType=true;	
				cube_add_parameter=true;
				timeSince = 0.0f;
			}


			if(ref == mGUI_CubeFaceOnTopDec && timeSince >= 0.2f) {
				faceOnTop--;
				cube_faceOnTop=true;					
				timeSince = 0.0f;
			}

			if(ref == mGUI_CubeFaceOnTopInc && timeSince >= 0.2f) {
				faceOnTop++;
				cube_faceOnTop=true;	
				timeSince = 0.0f;
			}

		    if(ref == mGUI_CubeFace1Dec && timeSince >= 0.2f&& hasTargetBody) {
				cube_Face1--;
				cube_add_parameter=true;
				timeSince = 0.0f;
			}

			if(ref == mGUI_CubeFace1Inc && timeSince >= 0.2f&& hasTargetBody) {
				cube_Face1++;
				cube_add_parameter=true;
				timeSince = 0.0f;	
				cube_Face1Inc=true;
			}

		  if(ref == mGUI_CubeFace2Dec && timeSince >= 0.2f) {
				cube_Face2--;
				cube_add_parameter=true;
				timeSince = 0.0f;
			}

			if(ref == mGUI_CubeFace2Inc && timeSince >= 0.2f) {
				cube_Face2++;	
				cube_add_parameter=true;
				cube_Face2Inc=true;
				timeSince = 0.0f;				
			}

			
			



//*****************************************************************************
			if(ref == mGUI_addCube && timeSince >= 1.0f){							
				cube_add=true;
				timeSince = 0.0f;
			}		

			if (ref == mGUI_deleteCube && timeSince >= 1.0f && hasTargetBody) {
				cube_deleteCube = true;
				timeSince = 0.0f;
			}

			if (ref == mGUI_restart && timeSince >= 1.0f) {
				cube_restart = true;
				cube_playEvolve = false;
				cube_getData = false;
				cube_evolve = false;
				record = false;
				cube_newC = true;				
				timeSince = 0.0f;
				mGUI_recordState->setValue("STOP");
				mGUI_playEvolveState->setValue("OFF");
				mGUI_evolveState->setValue("OFF");
                mGUI_getDataState->setValue("OFF");

				timeSince = 0.0f;
			}								

			if (ref == mGUI_evolve && timeSince >= 1.0f) {
				//cube_evolve = true;
				cube_evolve=!cube_evolve;
				if(cube_evolve) {
					mGUI_EvolutionWindow->show();
					mGUI_evolveState->setValue("ON");
					mGUI_EvolutionBest->setValue("0");
					mGUI_EvolutionAverage->setValue("0");
					cube_playEvolve = false;
					cube_getData = false;
					cube_newC = true;
				}
				else {
					mGUI_EvolutionWindow->hide();
					mGUI_evolveState->setValue("OFF");
					cube_restart = true;
				}
				mGUI_playEvolveState->setValue("OFF");
                mGUI_getDataState->setValue("OFF");
				timeSince = 0.0f;
			}

			if (ref == mGUI_playEvolve && timeSince >= 1.0f){
				//cube_playEvolve = true;
				cube_playEvolve=!cube_playEvolve;
				if(cube_playEvolve) {
					mGUI_playEvolveState->setValue("ON");					
					cube_getData = false;
					cube_evolve = false;
					cube_newC = true;
				}
				else {
					mGUI_playEvolveState->setValue("OFF");
					cube_restart = true;
				}
				mGUI_evolveState->setValue("OFF");
                mGUI_getDataState->setValue("OFF");
				timeSince = 0.0f;
			}

			if (ref == mGUI_getData && timeSince >= 1.0f){
				//cube_getData = true;
				cube_getData=!cube_getData;
				if(cube_getData) {
					mGUI_getDataState->setValue("ON");
					cube_playEvolve = false;
					cube_evolve = false;
					cube_newC = true;
				}
				else {
					mGUI_getDataState->setValue("OFF");
					cube_restart = true;
				}
				mGUI_playEvolveState->setValue("OFF");
				mGUI_evolveState->setValue("OFF");
				timeSince = 0.0f;
			}
//*****************************************************************************

			if (ref == mGUI_rotateClockwise && timeSince >= 0.2f && hasTargetBody) {
				mGUI_rotate->setValue("CCW");
				cube_rotateClockwise = true;
				timeSince = 0.0f;
			}

			if (ref == mGUI_rotateCounterClockwise && timeSince >= 0.2f && hasTargetBody) {
				mGUI_rotate->setValue("CC");
				cube_rotateCounterClockwise = true;
				timeSince = 0.0f;
			}

			if (ref == mGUI_haltRotation && timeSince >= 0.3f && hasTargetBody) {
				mGUI_rotate->setValue("HALT");
				cube_haltRotation = true;
				timeSince = 0.0f;
			}

			if (ref == mGUI_record && timeSince >= 0.5f) {
				record=1-record;
				if(record){
					mGUI_recordState->setValue("STAR");
					recordStart=true;
				}
				else{
					mGUI_recordState->setValue("STOP");
					recordStop=true;
				}
			 timeSince = 0.0f;
			}

			if (ref == mGUI_angleDec &&cube_edit&& timeSince >= 0.1f) {
				char str[4];
				cube_angle-=10;
				if(cube_angle<0)cube_angle=359;
				itoa(cube_angle/10,str,10);
				mGUI_angle->setValue(str);
				cube_angleDec = true;
				cube_add_parameter=true;	
				timeSince = 0.0f;
			}

			if (ref == mGUI_angleInc &&cube_edit&&  timeSince >= 0.1f) {
				char str[4];
				cube_angle+=10;
				cube_angle=cube_angle%3600;
				itoa(cube_angle/10,str,10);
				mGUI_angle->setValue(str);
				cube_add_parameter=true;
				cube_angleInc = true;
				timeSince = 0.0f;
			}

			if (ref == mGUI_angleDec10 && cube_edit&& timeSince >= 0.1f) {
				char str[4];
				cube_angle-=100;
				if(cube_angle<0)cube_angle=3600+cube_angle;
				itoa(cube_angle/10,str,10);
				mGUI_angle->setValue(str);
				cube_add_parameter=true;
				cube_angleDec = true;
				timeSince = 0.0f;
			}

			if (ref == mGUI_angleInc10 && cube_edit&& timeSince >= 0.1f) {
				char str[4];
				cube_angle+=100;
				cube_angle=cube_angle%3600;
				itoa(cube_angle/10,str,10);
				mGUI_angle->setValue(str);
				cube_add_parameter=true;
				cube_angleInc = true;
				timeSince = 0.0f;
			}


           if (ref == mGUI_CubeHightDec10 && timeSince >= 0.1f) {
				char str[6];
				height_star-=1 ;
				if(height_star<1)
				 height_star=1;
				gcvt(height_star,2,str);
				mGUI_CubeHight->setValue(str);
				timeSince = 0.0f;		
			}

			if (ref == mGUI_CubeHightDec && timeSince >= 0.1f) {
				char str[6];
				height_star-=0.1 ;
				if(height_star<1)
				 height_star=1;
				gcvt(height_star,2,str);		
				mGUI_CubeHight->setValue(str);
				timeSince = 0.0f;		
			}

           if (ref == mGUI_CubeHightInc && timeSince >= 0.1f) {
				char str[6];
				height_star+=0.1 ;
				gcvt(height_star,2,str);	
				mGUI_CubeHight->setValue(str);
				timeSince = 0.0f;		
			}

			if (ref == mGUI_CubeHightInc10 && timeSince >= 0.1f) {
				char str[6];
				height_star+=1 ;
				gcvt(height_star,2,str);	
				mGUI_CubeHight->setValue(str);
				timeSince = 0.0f;		
			}


			if (ref == mGUI_EvolutionExitButton)
				mGUI_EvolutionWindow->hide();


			if (ref == mGUI_ExitButton)
				mShutdownNow = true;


			if (ref == mGUI_PauseButton && timeSince >= 0.4f) { 
				if (nxOgre::world::getSingleton().isPaused()) {
					world::getSingleton().resume();
					mGUI_PauseButton->mmn = "bgui.pause";
					mGUI_PauseButton->mma = "bgui.pause.active";
				}
				else {
					world::getSingleton().pause();				
					mGUI_PauseButton->mmn = "bgui.play";
					mGUI_PauseButton->mma = "bgui.play.active";
				}
				timeSince = 0.0f;
			}

			if (ref == mGUI_DebugButton && timeSince >= 0.25f && objectHasBeenSelected) {
				world::getSingleton().showDebugOverlay(mCamera, mWindow);
				world::getSingleton().debug( world::getSingleton().isDebugRendering() ^ 1);

				if (world::getSingleton().isDebugRendering()) {
					mGUI_DebugButton->mmn = "bgui.debugon";
					mGUI_DebugButton->mma = "bgui.debugon.active";
				}
				else {
					mGUI_DebugButton->mmn = "bgui.debug";
					mGUI_DebugButton->mma = "bgui.debug.active";
				}

				timeSince = 0.0f;
			}

			// MY
			//if(ref == mGUI_addCubeNeg_Half && timeSince >= 0.2f) {
			//}


			GUIbuttonPressed(ref);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void captureInput() {
			mKeyboard->capture();
			mMouse->capture();
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool isKeyDown(Key k) {
			return mKeyboard->isKeyDown(keys[k]);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool keyPressed( const KeyEvent &arg ) {
			
			if (arg.text == 8) {
				mGUI->injectBackspace(mPointer->getLeft(), mPointer->getTop());
				return true;
			}

			if (arg.text < 32 || arg.text > 126)
				return true;

			string k;
			k = static_cast<char>(arg.text);
			mGUI->injectKey(k, mPointer->getLeft(), mPointer->getTop());
			return true;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool keyReleased( const KeyEvent &arg ) {
			//mGUI->injectKey(mKeyboard->getAsString((OIS::KeyCode) arg.text), mPointer->getLeft(), mPointer->getTop());
			return true;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		float getRMouseX() {
			const MouseState &ms = mMouse->getMouseState();
			
			return ms.X.rel;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		float getRMouseY() {
			const MouseState &ms = mMouse->getMouseState();			
			return ms.Y.rel;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		float getRMouseZ() {
			const MouseState &ms = mMouse->getMouseState();
			return ms.Z.rel;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Key getMouseButton() {
			const MouseState &ms = mMouse->getMouseState();
			
			if (ms.buttons == 1)
				return MOUSE_PRIMARY;
			else if (ms.buttons == 2)
				return MOUSE_ALT;
			else if (ms.buttons == 3)
				return MOUSE_BOTH;
			else if (ms.buttons == 4)
				return MOUSE_MIDDLE;
			else
				return MOUSE_NONE;
	
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		Ogre::OverlayContainer* createOverlay(Ogre::String name, Vector2 position, Vector2 dimensions, Ogre::String material = "", Ogre::String caption = "", bool autoAdd = true) {
			std::cout << name << std::endl;

			Ogre::String type = "Panel";
			if (caption != "") {
				type = "TextArea";
			}

			Ogre::OverlayContainer* e = static_cast<Ogre::OverlayContainer*>(Ogre::OverlayManager::getSingleton().createOverlayElement(type, name));

			e->setMetricsMode(Ogre::GMM_PIXELS);
			e->setLeft(position.x);
			e->setTop(position.y);
			e->setWidth(dimensions.x);
			e->setHeight(dimensions.y);

			if (material != "")
				e->setMaterialName(material);
			
			if (caption != "") {
				e->setCaption(caption);
				e->setParameter("font_name", "nxogrefont");
				e->setParameter("char_height", "16");
				e->setParameter("horz_align", "left");
			}

			if (autoAdd) {
				mOverlay->add2D(e);
				e->show();
			}

			return e;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		void createPlane(Ogre::String _name, Ogre::String _material, Ogre::Vector3 _pos, Ogre::Vector2 _size, Ogre::Vector2 _subdivisions = Ogre::Vector2(1,1)) {
			Plane _plane;
			_plane.normal = Vector3::UNIT_Y;
			_plane.d = 0;


			MeshManager::getSingleton().createPlane(_name + ".plane",
				ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				_plane,_size.x,_size.y,1,1,true,1,_subdivisions.x,_subdivisions.y,Vector3::UNIT_Z);
	        
			Entity *_entity = mSceneMgr->createEntity(_name + ".entity", _name + ".plane");
			_entity->setMaterialName(_material);
			_entity->setCastShadows(false);
			
			sg->addEntity(_entity, _pos);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		void CreateEntNode(Ogre::String _name, Ogre::Vector3 _pos) {
			Entity *_entity = mSceneMgr->createEntity(_name + ".entity", _name + ".mesh");
			_entity->setCastShadows(false);
			sg->addEntity(_entity, _pos);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		SceneNode* CreateNodeEntity(Ogre::String _name, Ogre::String ent, Ogre::Vector3 _pos) {
			SceneNode* n = mSceneMgr->getRootSceneNode()->createChildSceneNode(_name);
			Entity* e = mSceneMgr->createEntity(_name + ".ent", ent);
			n->attachObject(e);
			n->setPosition(_pos);
			return n;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		void Shutdown() {
			delete mGUI;
			ApplicationStop();
			mRoot->shutdown();
			delete mRoot;

#ifdef _DEBUG
		std::ifstream ifs("OgreLeaks.log");
		string line;
		unsigned int i=0;

		cout << "\n\n\nLeak Report" << std::endl;
		cout << "-----------" << std::endl;
		while( !ifs.eof() ) {
			i++;
			std::getline(ifs,line);
			
			if (i < 4)
				continue;

			if (line.length() == 0)
				continue;

			if (line.substr(0,1) == "-")
				continue;

			std::cout << "   ->" << line << std::endl;
		}
		cout << "\n\n\n";

#endif
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		bool frameStarted(const FrameEvent& evt) {
			
			if(mWindow->isClosed())
				return false;

			captureInput();

			if (isKeyDown(QUIT))
				mShutdownNow = true;
		
			if (mMouseMode == CAMERA_CONTROL) {
				mRotX = Degree(-getRMouseX() * 0.13);
				mRotY = Degree(-getRMouseY() * 0.13);
				mCamera->yaw(mRotX);
				mCamera->pitch(mRotY);
			}
			else if (mMouseMode == CAMERA_FORCE) {
				mRotX = Degree(-getRMouseX() * 0.13);
				mRotY = Degree(-getRMouseY() * 0.13);
				mCamera->yaw(mRotX);
				mCamera->pitch(mRotY);
			}
			else if (mMouseMode == PICK && getMouseButton() == MOUSE_PRIMARY) {
				float x,y;
				x=mPointer->getLeft() + getRMouseX();
				y=mPointer->getTop() + getRMouseY();
				if(x>mWindow->getWidth()) x=mWindow->getWidth();
				if(y>mWindow->getHeight()) y=mWindow->getHeight();
				if(x<0) x=0;
				if(y<0) y=0;
				mPointer->setPosition(x,y);
				
				mGUI->injectMouse(mPointer->getLeft(), mPointer->getTop(), true);
				//if (!mGUI->injectMouse(mPointer->getLeft(), mPointer->getTop(), true))
				//	mouseMoveOrGrab(mPointer->getLeft(), mPointer->getTop(), true);
			}
			else if (mMouseMode == PICK && getMouseButton() == MOUSE_ALT) {
				float x,y;
				x=mPointer->getLeft() + getRMouseX();
				y=mPointer->getTop() + getRMouseY();
				if(x>mWindow->getWidth()) x=mWindow->getWidth();
				if(y>mWindow->getHeight()) y=mWindow->getHeight();
				if(x<0) x=0;
				if(y<0) y=0;
				mPointer->setPosition(x,y);				

				mouseMoveOrGrab(mPointer->getLeft(), mPointer->getTop(), false);
			}
			else if (mMouseMode == PICK && getMouseButton() == MOUSE_BOTH) {

				mRotX = Degree(-getRMouseX() * 0.13);
				mRotY = Degree(-getRMouseY() * 0.13);
				mCamera->yaw(mRotX);
				mCamera->pitch(mRotY);
			}
			else if (mMouseMode == PICK) {
				float x,y;
				x=mPointer->getLeft() + getRMouseX();
				y=mPointer->getTop() + getRMouseY();
				if(x>mWindow->getWidth()) x=mWindow->getWidth();
				if(y>mWindow->getHeight()) y=mWindow->getHeight();
				if(x<0) x=0;
				if(y<0) y=0;
				mPointer->setPosition(x,y);		


				mGUI->injectMouse(mPointer->getLeft(), mPointer->getTop(), false);
			}

			if(getRMouseZ()!=0&&cube_edit&& timeSince >= 0.2f){
				if(getRMouseZ()>0){
				 cube_Face1++;
				 cube_Face1Inc=true;
				}
				else
				 cube_Face1--;
				cube_add_parameter=true;
			   timeSince = 0.0f;
			}
		
			if(getMouseButton()== MOUSE_MIDDLE&&targetBody!=NULL&&cube_edit&& timeSince >= 0.5f){
			   cube_add=true;
			   timeSince = 0.0f;
			}

			if(getMouseButton()== MOUSE_MIDDLE&&cube_rotation==true&&cube_rotation_begin==false&&cube_rotation_end==false&&targetBody!=NULL&&!cube_edit){	
				std::cout<< "MIDDLE 3"<<endl;
				targetBody->unFreezeAll();	
			    // for(int n=0;n<halfCubes.size()n++)
				//	halfCubes[n]->unfreezeAll();
				cube_rotation=false;			
				cube_rotation_end=true;
			}
			
			if(getMouseButton()!= MOUSE_MIDDLE&&cube_rotation==false&&cube_rotation_begin==false&&cube_rotation_end==true&&targetBody!=NULL&&!cube_edit){	
				std::cout<< "MIDDLE 4"<<endl;
				cube_rotation_end=false;
			    cube_rotation_begin=true;
			}			

			if(getMouseButton()== MOUSE_MIDDLE&&cube_rotation==false&&cube_rotation_begin==true&&cube_rotation_end==false&&targetBody!=NULL&&!cube_edit){	
				std::cout<< "MIDDLE 1"<<endl;							
				cube_rotation=true;	
				targetBody->freezeAll();
				//for(int n=0;n<halfCubes.size()n++)
				//	halfCubes[n]->freezeAll();
			}
						
			if(getMouseButton()!=MOUSE_MIDDLE && cube_rotation==true&&targetBody!=NULL&&!cube_edit){				
				cube_rotation_begin=false;			
				std::cout<< "MIDDLE 2"<<endl;
				pose cPose;
				Vector3 position;
				Quaternion quatern,q;		
				height_star+=getRMouseZ()/240;
				if(height_star<1) height_star=1;
				char str[10];	
				itoa(height_star,str,10);
				mGUI_CubeHight->setValue(str);

				targetBody->setGlobalPosition(0,0.6*height_star,0);	

				position = targetBody->getGlobalPosition();
				quatern =  targetBody->getGlobalOrientation();				

				Vector3 vx,vy,vz;
				q = NxTools::convert(cPose.q);
				vx = Vector3(q.xAxis().x, q.xAxis().y, q.xAxis().z);
				vx.normalise();
				vy = Vector3(q.yAxis().x, q.yAxis().y, q.yAxis().z);
				vy.normalise();
				vz = Vector3(q.zAxis().x, q.zAxis().y, q.zAxis().z);
				vz.normalise();				
				int x=getRMouseX();
				int y=getRMouseY();
				std::cout <<"getLeft"<<mPointer->getLeft()<<"getTop "<<mPointer->getTop()<<" X:"<< getRMouseX()<<" Y:"<< getRMouseY()<<" Z:"<<getRMouseZ()<<endl;
				
				cPose=pose(position,Quaternion(Degree(x%360),vy)*Quaternion(Degree(y%360),vz)*quatern);
				targetBody->setGlobalOrientation(cPose.q.w,cPose.q.x,cPose.q.y,cPose.q.z);	
				
				std::cout<<"MIDDLE"<<endl;
			}

			if (mMouseMode == PICK || mMouseMode == CAMERA_CONTROL) {

				if (isKeyDown(PAN_LEFT))
					mTranslateVector.x = -mMoveScale;

				if (isKeyDown(PAN_RIGHT))
					mTranslateVector.x = mMoveScale;

				if (isKeyDown(PAN_FORWARD))
					mTranslateVector.z = -mMoveScale;

				if (isKeyDown(PAN_BACKWARD))
					mTranslateVector.z = mMoveScale;

				if (isKeyDown(PAN_UP))
					mTranslateVector.y = mMoveScale;

				if (isKeyDown(PAN_DOWN))
					mTranslateVector.y = -mMoveScale;

				mCamera->moveRelative(mTranslateVector);
			}
		
			mMoveScale = 16 * evt.timeSinceLastFrame;
			
			mRotX = 0;
			mRotY = 0;
			mTranslateVector = Vector3::ZERO;

			timeSince += evt.timeSinceLastFrame;

/*			curTime = static_cast<unsigned int> (time(0));
			timeDiff = (curTime - timeAcc) > timeInter;
			totalTime = (curTime - iniTime)> expeTime;
*/			
			newFrame(evt.timeSinceLastFrame);
			
			//
			//		Pausing NxOgre
			//

			//if (isKeyDown(DEBUG_MODE) && timeSince >= 0.25f) {

			//	// Technically this only needs to be done once, and it should be in your setup code.
			//	world::getSingleton().showDebugOverlay(mCamera, mWindow);
			//	
			//	// Pause or unpause the DebugRenderer
			//	world::getSingleton().debug( world::getSingleton().isDebugRendering() ^ 1);




			//	if (world::getSingleton().isDebugRendering()) {
			//		mGUI_DebugButton->mmn = "bgui.debugon";
			//		mGUI_DebugButton->mma = "bgui.debugon.active";
			//	}
			//	else {
			//		mGUI_DebugButton->mmn = "bgui.debug";
			//		mGUI_DebugButton->mma = "bgui.debug.active";
			//	}

			//	timeSince = 0.0f;
			//}

			if (isKeyDown(SCREENSHOT) && timeSince >= 0.25f) {
				mWindow->writeContentsToFile("screenshot." + Ogre::StringConverter::toString(Ogre::Math::Floor(mRoot->getTimer()->getMilliseconds() / 1000))
					+ "." + Ogre::StringConverter::toString(Ogre::Math::Floor(mRoot->getTimer()->getMilliseconds() % 1000 / 100)) + ".png");
				timeSince = 0.0f;
			}

			//if (isKeyDown(FASTER) && timeSince >= 0.25f) {
			//	speed *= 2;
			//	world::getSingleton().setTimeModifer(speed);


			//	timeSince = 0.0f;

			//	if (speed > 4)
			//		speed = 4;
			//	
			//	if (speed != 0) {
			//		gui_slowfast->show();
			//		gui_slowfast->setCaption(Ogre::StringConverter::toString(speed));
			//	} else
			//		gui_slowfast->hide();

			//}

			//if (isKeyDown(SLOWER) && timeSince >= 0.25f) {
			//	speed /= 2;
			//	world::getSingleton().setTimeModifer(speed);
			//	timeSince = 0.0f;

			//	if (speed < 0.25)
			//		speed = 0.25;

			//	if (speed != 0) {
			//		gui_slowfast->show();
			//		gui_slowfast->setCaption(Ogre::StringConverter::toString(speed));
			//	} else
			//		gui_slowfast->hide();

			//}

			if (isKeyDown(PAUSE) && timeSince >= 0.5f) {

				if (nxOgre::world::getSingleton().isPaused()) {
					world::getSingleton().resume();
					gui_pause->hide();
				}
				else {
					world::getSingleton().pause();
					gui_pause->show();
				}
				timeSince = 0.0f;

			}


			if (firstFrame) {
				if (mMouseMode == PICK) {
					mCamera->setPosition(10,10,10); 
					mCamera->lookAt(0,1,0);
				}
				firstFrame = false;
			}

			if (hasTargetBody == false) {
				targetNode->setVisible(false);
	
			} else {
				Vector3 p = targetBody->getGlobalPosition();
				p.y = 0.01;
				targetNode->setPosition(p);	
				targetNode->setVisible(true);

			}
			return true;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		Ogre::String mmToString(mouseMode mm) {
			
			switch(mm) {

				case CAMERA_FOLLOW:
					return "Camera (Following)";
				break;

				case CAMERA_CONTROL:
					return "Camera (Free)";
				break;

				case PICK:
					return "Mouse picking";
				break;

			};

			return "?";

		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool frameEnded(const FrameEvent& evt) {
			
			if (mShutdownNow)
				return false;
				
			return true;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void ApplicationStart() {

			mCamera->setPosition(10,10,10);
			mCamera->lookAt(0,0,0);

			targetBody = 0;
			hasTargetBody = false;

			targetNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("target");

			//Entity *_entity =  mSceneMgr->createEntity("nx.bodyguide.ent", "nx.bodyguide.mesh");
			//_entity->setCastShadows(false);
			//targetNode->attachObject(_entity);
			targetNode->scale(0.5,0.5,0.5);
			targetNode->setVisible(false);
			
			firstFrame = true;
			timeSince = 0;
			speed = 1;

			prestart();
			start();

			mRayCaster = world::getSingleton().getScene("Main")->createRayCaster("MousePicker", Vector3::ZERO, Vector3::ZERO, 200);
			
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void ApplicationStop() {
			prestop();
			stop();
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void getSetup() {
			getTutorialSettings();
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		void mouseMoveOrGrab(float x, float y, bool move_or_grab) {

			Ogre::Ray r = mCamera->getCameraToViewportRay(
				float(x / mWindow->getWidth()),
				float(y / mWindow->getHeight())
			);

			mRayCaster->setOrigin(r.getOrigin());
			mRayCaster->setDirection(r.getDirection());

			if (!mRayCaster->cast()) {
				return;
			}

			// Move it
			//if (move_or_grab) {
				
				//if (hasTargetBody) {

				//	Ogre::Plane p;
				//	p.normal = r.getDirection();
				//	p.d = -mRayCaster->mHitPos.dotProduct(p.normal);

				//	std::pair<bool, Real> res;
				//	res = r.intersects(p);
				//	Vector3 objPoint;

					//if (res.first) {

					//	Vector3 force = r.getPoint(res.second) - targetBody->getGlobalPosition();
					//	force *= 10;
					//	force -= NxTools::convert(targetBody->mActor->getPointVelocity(NxTools::convert(targetBody->getGlobalPosition())));
					//	targetBody->addForceAtPos(force, targetBody->getGlobalPosition());

					//}

				//}

			//	return;
			//}

			// Grab it

			if (!move_or_grab) {

				body *t = mRayCaster->mHitBody;
				//if(hasTargetBody == false) {
				//	if(targetBody!=0) targetBody->mNode->showBoundingBox(false);
				//}
				if (t->isStatic()) {
					targetBody = 0;
					hasTargetBody = false;
					return;
				}

				// Can't add forces to Kinematics.
				if (t->isKinematic())
					return;

				if (t == targetBody)
					return;
				
				// Hide bounding box for previously selected object
				if(targetBody!=0) {targetBody->mNode->showBoundingBox(false);}
				cube_add_parameter=true;
				targetBody = t;
				hasTargetBody = true;
				objectHasBeenSelected = true;
			}

		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		virtual void getTutorialSettings() = 0;
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void prestart() = 0;
		virtual void prestop() = 0;
		virtual void newFrame(float _time) = 0;
		virtual void GUIbuttonPressed(BetaGUI::Button *ref) {};
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		// Tutorial bits
		Ogre::String mTutorialName;
		Ogre::String mTutorialDescription;
		bool mShutdownNow;
		bool firstFrame;
		float timeSince;
		bool mConstruct;		

		// Ogre pointer bits
		Ogre::Root* mRoot;
		Ogre::RenderSystem* mRenderSystem;
		Ogre::Camera* mCamera;
		Ogre::RenderWindow* mWindow;
		Ogre::Viewport* mViewport;
		Ogre::SceneManager* mSceneMgr;
		int winHeight;
		int winWidth;

		// Gui bits
		OverlayElement* gui_pause;
		OverlayElement* gui_slowfast;

		// Settings
		std::map<Ogre::String, Ogre::String> settings;
		std::map<Key, KeyCode> keys;
		float speed;

		// Mouse movement bits.
		Vector3 mTranslateVector;
		float mMoveScale;
		Radian mRotX;
		Radian mRotY;
		Radian mRotZ;

		mouseMode mMouseMode;

		// Overlay, Pointer and GUI bits
		Ogre::Overlay* mOverlay;
		Ogre::OverlayContainer* mPointer;
		Ogre::OverlayContainer* mTargetCaption;
		Ogre::StaticGeometry* sg;

		// Mousepicker and Raycasting bits
		body *targetBody;
		SceneNode* targetNode;
		bool hasTargetBody;
		bool objectHasBeenSelected;
		rayCaster* mRayCaster;		

		// Input bits
		OIS::Mouse*    mMouse;
		OIS::Keyboard* mKeyboard;

		bool cube_rotateClockwise;
		bool cube_rotateCounterClockwise;
		bool cube_haltRotation;
		bool cube_angleDec;
		bool cube_angleInc;

		bool cube_typeCube;
		
		bool cube_add;
		bool cube_addCubePosY;
		bool cube_addCubeNegY;
		bool cube_addCubePosZ;
		bool cube_addCubeNegZ;
		bool cube_addCubePosX;
		bool cube_addCubeNegX;
		bool cube_deleteCube;
		bool cube_restart;
		bool cube_evolve;
		bool cube_playEvolve;
		bool cube_getData;
		bool cube_newC;
		bool cube_orientationType;
		int cube_orientation;
		bool cube_faceOnTop;
		char faceOnTop;
		int cube_angle;
		int cube_angleDisplay;
		signed char cube_typeCubeID;
		unsigned char cube_typeCubeClass;
		float height_star;
		char cube_addFace;		
		bool cube_rotation_begin;
		bool cube_rotation_end;
		bool cube_rotation;
		bool saveCube;
		bool loadCube;
		bool recordStart;
		bool record;
		bool recordStop;
		bool cube_add_parameter;
		signed char cube_Face1;
		signed char cube_Face2;
		
		bool cube_Face1Inc;
		bool cube_Face2Inc;
		bool cube_edit;

		Ogre::OverlayContainer* mCaption1;
		Ogre::OverlayContainer* mCaption2;
		Ogre::OverlayContainer* mCaption3;

		// GUI
		BetaGUI::GUI*	 mGUI;

		BetaGUI::TextInput* mGUI_editState;
		BetaGUI::Button* mGUI_edit;
		BetaGUI::Button* mGUI_ExitButton;
		BetaGUI::Button* mGUI_PauseButton;
		BetaGUI::Button* mGUI_DebugButton;
		BetaGUI::Window* mGUI_ExitWindow;

		BetaGUI::Window* mGUI_InstructionWindow;
		BetaGUI::TextInput* mGUI_CubeOrientation;
		BetaGUI::Button* mGUI_CubeOrientationInc;
		BetaGUI::Button* mGUI_CubeOrientationDec;
		
		BetaGUI::TextInput* mGUI_CubeFaceOnTop;
		BetaGUI::Button* mGUI_CubeFaceOnTopInc;
		BetaGUI::Button* mGUI_CubeFaceOnTopDec;

		BetaGUI::Window* mGUI_ControlWindow;

		BetaGUI::TextInput* mGUI_rotate;
		BetaGUI::Button* mGUI_rotateClockwise;
		BetaGUI::Button* mGUI_rotateCounterClockwise;
		BetaGUI::Button* mGUI_record;
		BetaGUI::TextInput* mGUI_recordState;

		BetaGUI::Button* mGUI_haltRotation;
		BetaGUI::Button* mGUI_angleDec10;		
		BetaGUI::Button* mGUI_angleDec;
		BetaGUI::Button* mGUI_angleInc;
		BetaGUI::Button* mGUI_angleInc10;
		BetaGUI::TextInput* mGUI_angle;

		BetaGUI::Button* mGUI_setAngleDec10;		
		BetaGUI::Button* mGUI_setAngleDec;
		BetaGUI::Button* mGUI_setAngleInc;
		BetaGUI::Button* mGUI_setAngleInc10;
		BetaGUI::TextInput* mGUI_setAngle;		

		BetaGUI::TextInput* mGUI_typeCube;
		BetaGUI::Button* mGUI_typeCubeDec;
		BetaGUI::Button* mGUI_typeCubeInc;

		BetaGUI::TextInput* mGUI_face;
		BetaGUI::Button* mGUI_faceDec;
		BetaGUI::Button* mGUI_faceInc;
		
		BetaGUI::TextInput* mGUI_CubeFace1;
		BetaGUI::Button* mGUI_CubeFace1Dec;
		BetaGUI::Button* mGUI_CubeFace1Inc;
		
		BetaGUI::TextInput* mGUI_CubeFace2;
		BetaGUI::Button* mGUI_CubeFace2Dec;
		BetaGUI::Button* mGUI_CubeFace2Inc;

		BetaGUI::Button* mGUI_addCube;
		BetaGUI::Button* mGUI_addCubePosY;
		BetaGUI::Button* mGUI_addCubeNegY;
		BetaGUI::Button* mGUI_addCubePosZ;
		BetaGUI::Button* mGUI_addCubeNegZ;
		BetaGUI::Button* mGUI_addCubePosX;
		BetaGUI::Button* mGUI_addCubeNegX;
		BetaGUI::Button* mGUI_deleteCube;


		//////////////////////////////////////////////////////////-----------------------
		
		BetaGUI::TextInput* mGUI_CubeHight;		
		BetaGUI::Button* mGUI_CubeHightInc10;
		BetaGUI::Button* mGUI_CubeHightInc;
		BetaGUI::Button* mGUI_CubeHightDec10;
		BetaGUI::Button* mGUI_CubeHightDec;

		BetaGUI::Button* mGUI_restart;
	
		BetaGUI::Button* mGUI_evolve;
		BetaGUI::TextInput* mGUI_evolveState;
		BetaGUI::Button* mGUI_playEvolve;
		BetaGUI::TextInput* mGUI_playEvolveState;
		BetaGUI::Button* mGUI_getData;
		BetaGUI::TextInput* mGUI_getDataState;
		
		BetaGUI::Window* mGUI_SaveLoadWindow;
		BetaGUI::Button* mGUI_saveCube;
		BetaGUI::Button* mGUI_loadCube;
		BetaGUI::TextInput* mGUI_fileName;

		BetaGUI::Window* mGUI_EvolutionWindow;
		BetaGUI::TextInput* mGUI_EvolutionGeneration;
		BetaGUI::TextInput* mGUI_EvolutionIndividu;
		BetaGUI::TextInput* mGUI_EvolutionTime;
		BetaGUI::TextInput* mGUI_EvolutionBest;
		BetaGUI::TextInput* mGUI_EvolutionAverage;
		BetaGUI::TextInput* mGUI_EvolutionPopulation;
		BetaGUI::Button* mGUI_EvolutionExitButton;
};

#endif