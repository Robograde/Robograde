/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include <memory/Alloc.h>
#include "NLayer.h"
#include "gui/GUIEngine.h"
#include "stdafx.h"

class NeuralNet
{
    
public:
	~NeuralNet();
    NeuralNet(int nIns,int nOuts,int nHiddenLays,int nNodesinHiddenLays,int netID);
    void Init();
    
    //access methods    
    void Use(rVector<float> &inputs,rVector<float> &outputs);
    void Train(rVector<float> &inputs,rVector<float> &outputs);
    float GetError()    {return m_Error;}
    void WriteWeights();
    void ReadWeights();
#if AI_DRAW_NEURAL_NETS == 1
	void SetText(rString text);
#endif
protected:
    //internal methods
    void AddLayer(int nNeurons,int nInputs,int type);
    void SetInputs(rVector<float>& inputs);
    void FindError(rVector<float>& outputs);
    void Propagate();
    void BackPropagate();


	//data
    rVector<NLayer*>  m_Layers;
    NLayer*         m_InputLayer;
    NLayer*         m_OutputLayer;

    float           m_LearningRate;
	float           m_Momentum;
	float           m_Error;

    int             m_NumInputs;
    int             m_nOutputs;
    int             m_NumLayers;
    int             m_nHiddenNodesperLayer;

	int				m_ID;

#if AI_DRAW_NEURAL_NETS == 1
	rVector<GUI::Sprite*> m_NeuronSprites;
	GUI::Text* m_Text;
#endif
};

