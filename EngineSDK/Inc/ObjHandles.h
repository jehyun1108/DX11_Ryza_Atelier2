#pragma once

NS_BEGIN(Engine)

struct ENGINE_DLL ObjHandles
{
    Handle transform;
    Handle animator;
    Handle mouth;
    Handle face;

    Handle camera;  
    Handle model;    
    Handle light;    
    Handle socket;   
    Handle freeCam;  
};

NS_END