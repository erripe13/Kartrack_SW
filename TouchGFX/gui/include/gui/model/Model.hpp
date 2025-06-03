#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

class Model
{
public:
	float getVitesse();
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();
protected:
    ModelListener* modelListener;
};

#endif // MODEL_HPP
