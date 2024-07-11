#pragma once
#include "ShapeManager.h"

class Disk: public ShapeManager {
private:
	int stride;
	int padding = 1;
public:
	Disk(GLFWwindow* window, int bindWidth, int bindHeight, int posX, int posY);
	~Disk();
	void addShape (Shape& s) override;
	void setTranslation(float x, float y) override;
};
