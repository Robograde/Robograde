/**************************************************
Viktor Kelkkanen
***************************************************/

#include "NLayer.h"
/*#include "utility.h"*/
#include <math.h>
#include <utility/Randomizer.h>


//---------------------------------------------------------
NLayer::NLayer(int nNeurons,int nInputs,int type):
m_Type(type)
{
    for (int i=0; i<nNeurons; ++i)
    {
        Neuron* newNeuron = pNew(Neuron);

        //each neuron has nInputs+1 weights, extra one is the bias value
        for (int j=0; j<nInputs+1; ++j)
        {
            newNeuron->Weights.push_back(g_Randomizer.SimRand(0,1000)*0.001f);
            newNeuron->LastDelta.push_back(0.0f);
        }
        //initial values
        newNeuron->Output = 0.0f;
        newNeuron->Error  = 999999.9f;

        m_Neurons.push_back(newNeuron);
    }
}

NLayer::~NLayer()
{
	for (int i = static_cast<int>(m_Neurons.size()) - 1; i > -1; i--)
	{
		pDelete(m_Neurons[i]);
		m_Neurons.erase(m_Neurons.begin() + i);
	}
}

void NLayer::Propagate(NLayer* nextLayer)
{
    int weightIndex;
	for (int i = 0; i<nextLayer->m_Neurons.size(); ++i)
    {
        weightIndex = 0;
        float value = 0.0f;
        
        int	numWeights = static_cast<int>(m_Neurons.size());
        for (int j=0; j<numWeights; ++j)
        {
            //sum the (weights * inputs), the inputs are the outputs of the prop layer
			value += nextLayer->m_Neurons[i]->Weights[j] * m_Neurons[j]->Output;
        }
        
        //add in the bias (always has an input of -1)
		value += nextLayer->m_Neurons[i]->Weights[numWeights] * -1.0f;

		nextLayer->m_Neurons[i]->Output = ActBipolarSigmoid(value);
    }

    return;
}

void NLayer::BackPropagate(NLayer* nextLayer)
{
    float outputVal, error;

	for (int i = 0; i<nextLayer->m_Neurons.size(); ++i)
    {
		outputVal = nextLayer->m_Neurons[i]->Output;
        error = 0;
        for (unsigned int j=0; j<m_Neurons.size(); ++j)
            error += m_Neurons[j]->Weights[i] * m_Neurons[j]->Error;

		float errorFunction = DerBipolarSigmoid(outputVal);

		nextLayer->m_Neurons[i]->Error = errorFunction*error;
    }
}

void NLayer::AdjustWeights(NLayer* inputs,float lrate, float momentum)
{
    for (unsigned int i=0; i<m_Neurons.size(); ++i)
    {
        int numWeights = static_cast<int>(m_Neurons[i]->Weights.size());
        for (int j=0; j<numWeights; ++j)
        {
            //bias weight always uses -1 output value
			float output = (j == numWeights - 1) ? -1 : inputs->m_Neurons[j]->Output;
            float error  = m_Neurons[i]->Error;
            float delta  = momentum*m_Neurons[i]->LastDelta[j] + (1-momentum)*lrate * error * output;

            m_Neurons[i]->Weights[j]   += delta;
            m_Neurons[i]->LastDelta[j]  = delta;
        }
    }
}

float NLayer::ActBipolarSigmoid(float x)
{
    return ((2/(1+exp(-x)))-1);
}

float NLayer::DerBipolarSigmoid(float x)
{
	return static_cast<float>(0.5 * (1 + ((2 / (1 + exp(-x * 1))) - 1))*(1 - ((2 / (1 + exp(-x * 1))) - 1)));
}

float NLayer::DerDerBipolarSigmoid(float x)
{
	return static_cast<float>(-((2 * exp(x)*(exp(x) - 1)) / pow((exp(x) + 1), 3)));
}

void NLayer::SaveDebugData(int layer)
{
	FILE* pFile;
	rString filePath = "../../../asset/ai/debuginfo_weights"+rToString(layer)+".txt";
	if ((pFile = fopen(filePath.c_str(), "w")) == NULL)
		return;

	for (int i = 0; i < m_Neurons.size(); i++)
	{
		fprintf(pFile, "\nNeuron: %d\n", i);

		for (int j = 0; j < m_Neurons[i]->Weights.size(); j += 1)
		{
			fprintf(pFile, "\n%d	", j);
			for (int k = j; k < m_Neurons[i]->DebugData.size(); k += 8)
				fprintf(pFile, "%f	", m_Neurons[i]->DebugData[k]);
		}
	}
	

	fclose(pFile);
}