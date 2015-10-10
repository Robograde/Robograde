/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#pragma once
#include <memory/Alloc.h>

enum
{
    NLT_INPUT,
    NLT_HIDDEN,
    NLT_OUTPUT
};

struct Neuron
{
    rVector<float> Weights;   //connection strengths
    rVector<float> LastDelta;//used for inertia in updating the weights while learning
    float Output;           //the fired potential of the neuron
    float Error;            //the error gradient of the potential from the expected 
                              //potential; used when learning
	rVector<float> DebugData;
};

class NLayer
{
public:
	~NLayer();
    NLayer(int nNeurons, int nInputs, int type = NLT_INPUT);
    void Propagate(NLayer* nextLayer);
    void BackPropagate(NLayer* nextLayer);
    void AdjustWeights(NLayer* inputs,float lrate = 0.1f, float momentum = 0.9f);
    
    float ActBipolarSigmoid(float value);

    float DerBipolarSigmoid(float value);
	float DerDerBipolarSigmoid(float value);
	void SaveDebugData(int layer);
	//data
    rVector<Neuron*> m_Neurons;
	int				m_Type;
};