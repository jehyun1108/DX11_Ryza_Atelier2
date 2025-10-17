#pragma once

#define GAME   GameInstance::GetInstance()
#define DC     GameInstance::GetInstance().GetContext()
#define DEVICE GameInstance::GetInstance().GetDevice() 

#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT       1
#define LIGHT_SPOT        2

#define MAX_BONES         1024

//#define HR(r) if(FAILED(r)) return E_FAIL
#define HR(r) if(FAILED(r)) assert(false);

#define NFD_IMPLEMENTATION

#define ENUM(r) static_cast<unsigned int>(r)

//#define ENUM(CLASS) static_cast<unsigned int>(CLASS)

#define MSG_BOX(_msg) MessageBox(NULL,TEXT(_msg),L"System Message", MB_OK)

#define NS_BEGIN(NAMESPACE) namespace NAMESPACE {
#define NS_END              }

#define USING(NAMESPACE) using namespace NAMESPACE;

#ifdef  ENGINE_EXPORTS
#define ENGINE_DLL _declspec(dllexport)
#else
#define ENGINE_DLL _declspec(dllimport)
#endif 