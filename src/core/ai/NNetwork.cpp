/**************************************************
2015 Viktor Kelkkanen
***************************************************/

#include "NNetwork.h"
#include <math.h>
#include "glm/detail/func_common.hpp"


NeuralNet::~NeuralNet()
{
	for (int i = static_cast<int>(m_Layers.size()) - 1; i > -1; i--)
	{
		//m_Layers[i]->SaveDebugData((1+m_ID)*10+i);

		pDelete(m_Layers[i]);
		m_Layers.erase(m_Layers.begin() + i);
	}
}



NeuralNet::NeuralNet(int nIns, int nOuts, int nHiddenLays, int nNodesPerHLays, int netID)
{
    m_NumInputs				= nIns;
    m_nOutputs				= nOuts;
    m_NumLayers				= nHiddenLays + 2;//plus the output layer
    m_nHiddenNodesperLayer	= nNodesPerHLays;
	m_ID					= netID;

    Init();
}


void NeuralNet::AddLayer(int nNeurons,int nInputs,int type)
{
    m_Layers.push_back(pNew(NLayer,nNeurons,nInputs,type));

}


void NeuralNet::Init()
{
    m_InputLayer   = NULL;
    m_OutputLayer  = NULL;
    m_Momentum     = 0.01f;
    m_LearningRate = 0.01f;

    //input layer
    AddLayer(m_NumInputs, 1, NLT_INPUT);
    
    if(m_NumLayers > 2)//multilayer network
    {
        //first hidden layer connect back to inputs
        AddLayer(m_nHiddenNodesperLayer, m_NumInputs, NLT_HIDDEN);
        
        //any other hidden layers connect to other hidden outputs
        //-3 since the first layer was the inputs,
        //the second (connected to inputs) was initialized above,
        //and the last one (connect to outputs) will be initiallized below
        for (int i=0; i<m_NumLayers-3; ++i)
            AddLayer(m_nHiddenNodesperLayer, m_nHiddenNodesperLayer, NLT_HIDDEN);
        
        //the output layer also connects to hidden outputs
        AddLayer(m_nOutputs, m_nHiddenNodesperLayer, NLT_OUTPUT);
    }
    else//perceptron
    {
        //output layer connects to inputs
        AddLayer(m_nOutputs, m_NumInputs, NLT_OUTPUT);
    }

#if AI_DRAW_NEURAL_NETS == 1
		
		int x = 200;
		int y = 270;
		rString netName = "NN" + rToString(m_ID);
		g_GUI.AddWindow(netName, GUI::Rectangle(x * m_ID, 0, x, y), "RootWindow", true)->SetVisible(true);
		g_GUI.OpenWindow(netName);
		g_GUI.UseFont(FONT_ID_LEKTON_8);
		m_Text = g_GUI.AddText(netName + "text", GUI::TextDefinition(netName.c_str(), m_ID * x, y + 10, glm::vec4(1.0f)), "RootWindow");
		m_Text->GetTextDefinitionRef().Text = netName;
		for (int i = 0; i < m_Layers.size(); i++)
		{
			for (int j = 0; j < m_Layers[i]->m_Neurons.size(); j++)
			{
				m_NeuronSprites.push_back(g_GUI.AddSprite(netName + rToString(i) + "_" + rToString(j), GUI::SpriteDefinition("../../../asset/gui/sprites/minimap/controlPoint.png", i * 32 + 32, j * 32 + 32, 8, 8), netName));
				m_NeuronSprites.back()->SetVisible(true);
				m_NeuronSprites.back()->GetSpriteDefinitionRef().Colour.g = 1;
				m_NeuronSprites.back()->GetSpriteDefinitionRef().Colour.b = 1;
			}
		}
#endif

    m_InputLayer = m_Layers[0];
    m_OutputLayer= m_Layers[m_NumLayers-1];
}


void NeuralNet::Propagate()
{
	for (int i = 0; i < m_NumLayers - 1; ++i)
	{
		m_Layers[i]->Propagate(m_Layers[i + 1]);
	}

#if AI_DRAW_NEURAL_NETS == 1
	for (int i = 0; i < m_NumLayers; ++i)
	{
		for (int j = 0; j < m_Layers[i]->m_Neurons.size(); j++)
		{
			int index = i*m_NumInputs + j;

			if (index > m_NeuronSprites.size())
				continue;

			int neuronOut = static_cast<int>(((m_Layers[i]->m_Neurons[j]->Output)+1.0f));

			if (i == m_NumLayers - 1)
				neuronOut *= 8;

			if (neuronOut < 0)
			{
				neuronOut = -neuronOut;
				m_NeuronSprites[index]->GetSpriteDefinitionRef().Colour.b = 1;
				m_NeuronSprites[index]->GetSpriteDefinitionRef().Colour.g = 0;
			}
			else if (neuronOut > 0)
			{
				m_NeuronSprites[index]->GetSpriteDefinitionRef().Colour.g = 1;
				m_NeuronSprites[index]->GetSpriteDefinitionRef().Colour.b = 0;
			}
			else
			{
				m_NeuronSprites[index]->GetSpriteDefinitionRef().Colour.g = 1;
				m_NeuronSprites[index]->GetSpriteDefinitionRef().Colour.b = 1;
				neuronOut = 8;
			}

			if (neuronOut > 32)
				neuronOut = 32;

			m_NeuronSprites[index]->GetSpriteDefinitionRef().Width = neuronOut;
			m_NeuronSprites[index]->GetSpriteDefinitionRef().Height = neuronOut;
			m_NeuronSprites[index]->GetSpriteDefinitionRef().CenterX = true;
			m_NeuronSprites[index]->GetSpriteDefinitionRef().CenterY = true;
			m_NeuronSprites[index]->GetSpriteDefinitionRef().Colour.r = m_Layers[i]->m_Neurons[j]->Error;

		}
	}
#endif
}


void NeuralNet::BackPropagate()
{
    //backprop the error
    for (int i=m_NumLayers-1; i>0; --i)
		m_Layers[i]->BackPropagate(m_Layers[i - 1]);
    
    //adjust the weights
    for (int i=1; i<m_NumLayers; i++)
		m_Layers[i]->AdjustWeights(m_Layers[i - 1], m_LearningRate, m_Momentum);
}


void NeuralNet::Train(rVector<float> &inputs,rVector<float> &outputs)
{
    SetInputs(inputs);
    Propagate();
	m_Error = 0;
    FindError(outputs);
	if (m_Error > 0.00f)
	{
		BackPropagate();
	}
}


void NeuralNet::Use(rVector<float> &inputs,rVector<float> &outputs)
{
    SetInputs(inputs);
    Propagate();

    for(unsigned int i =0;i< m_OutputLayer->m_Neurons.size();++i)
		outputs[i] = m_OutputLayer->m_Neurons[i]->Output;
}

void NeuralNet::SetInputs(rVector<float>& inputs)
{
	for (int i = 0; i<m_InputLayer->m_Neurons.size(); ++i)
		m_InputLayer->m_Neurons[i]->Output = inputs[i];
}

void NeuralNet::FindError(rVector<float>& outputs)
{
	for (int i = 0; i<m_OutputLayer->m_Neurons.size(); ++i)
    {
        float outputVal = m_OutputLayer->m_Neurons[i]->Output;
		float error = outputs[i] - outputVal;

		float errorFunction = m_OutputLayer->DerBipolarSigmoid(outputVal);

		m_OutputLayer->m_Neurons[i]->Error = errorFunction*error;

		m_Error += std::abs(m_OutputLayer->m_Neurons[i]->Error);
    }
}

void NeuralNet::WriteWeights()
{
    FILE* pFile;
	rString filePath = "../../../asset/ai/bestWeights.txt";
	if ((pFile = fopen(filePath.c_str(), "w")) == NULL)
        return;
    
    for(int i = 0;i<m_NumLayers;++i)
    {
		for (int j = 0; j<m_Layers[i]->m_Neurons.size(); ++j)
        {
			int numWeights = static_cast<int>(m_Layers[i]->m_Neurons[j]->Weights.size());
            for(int k=0;k< numWeights;++k)
				fprintf(pFile, "%f ", m_Layers[i]->m_Neurons[j]->Weights[k]);
        }
    }
    fclose(pFile);
}

void NeuralNet::ReadWeights()
{
    FILE* pFile;
	rString filePath = "../../../asset/ai/bestWeights.txt";
	if ((pFile = fopen(filePath.c_str(), "r")) == NULL)
	{
		printf("Failed to read neural net weights. Weights are now initialized with random\n");
		return;
	}
    
    for(int i = 0;i<m_NumLayers;++i)
    {
		int numNeurons = static_cast<int>(m_Layers[i]->m_Neurons.size());
        for(int j =0;j<numNeurons;++j)
        {
			int numWeights = static_cast<int>(m_Layers[i]->m_Neurons[j]->Weights.size());
            for(int k=0;k< numWeights;++k)
				fscanf(pFile, "%f ", &m_Layers[i]->m_Neurons[j]->Weights[k]);
        }
    }
    fclose(pFile);
}

#if AI_DRAW_NEURAL_NETS == 1
void NeuralNet::SetText(rString text)
{
	m_Text->GetTextDefinitionRef().Text = text;
}
#endif