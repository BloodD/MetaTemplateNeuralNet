#pragma once
class SimpleNeuron
{
public:
	SimpleNeuron();
	SimpleNeuron(int, int);
	~SimpleNeuron();
	float GetOutput();
	int GetLayer();
	int GetIndex();
	void SetOutput(float);
private:
	float m_Output;
	int m_Layer;
	int m_Index;
};

