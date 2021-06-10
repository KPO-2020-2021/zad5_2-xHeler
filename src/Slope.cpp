#include "Slope.h"

Slope::Slope(double lenght, double height, double width) {
    Vector v({lenght, 0, 0});
    Vector r({0, 0, width});
    centerOfSymmetry = Vector();

    color = DEFAULT_COLOR;
    basis = Polygon(v, r, Vector(), color);
    basis.translation(-lenght / 2, 0 , - width /2);

    triangles[0] = Polygon<TRIANGLE>({basis[0], basis[1], basis[1] + Vector({0, height, 0})});
    triangles[1] = Polygon<TRIANGLE>({basis[2], basis[3], basis[2] + Vector({0, height, 0})});

    rectangles[0] = Polygon<RECTANGLE>({basis[0], basis[1] + Vector({0, height, 0}),
                                        basis[2] + Vector({0, height, 0}), basis[3]});
    rectangles[1] = Polygon<RECTANGLE>({basis[1], basis[1] + Vector({0, height, 0}),
                                        basis[2] + Vector({0, height, 0}), basis[2]})
                                                ;
}

void Slope::draw() {
    basis.draw();
    for (int i = 0; i < SIDES_W; ++i) {
        triangles[i].draw();
        rectangles[i].draw();
    }
}

void Slope::translation(Vector<STANDARD_SIZE> translationVector) {
    centerOfSymmetry = centerOfSymmetry + translationVector;
    basis.translation(translationVector);
    for (int i = 0; i < SIDES_W; ++i) {
        triangles[i].translation(translationVector);
        rectangles[i].translation(translationVector);
    }
}

void Slope::rotationMain(Vector<STANDARD_SIZE> rotations) {
    centerOfSymmetry.rotateByEulerAngles(rotations[0], rotations[1], rotations[2]);
    basis.rotationMain(rotations);
    for (int i = 0; i < SIDES_W; ++i) {
        triangles[i].rotationMain(rotations);
        rectangles[i].rotationMain(rotations);
    }
}

void Slope::rotationLocal(Vector<STANDARD_SIZE> rotations) {
    Vector tmp = centerOfSymmetry * -1;
    translation(tmp);
    rotationMain(rotations);
    tmp = tmp * -1;
    translation(tmp);
    centerOfSymmetry = tmp;
}

void Slope::setColor(std::string _color) {
    color = _color;
    basis.setColor(color);
    for (int i = 0; i < SIDES_W; ++i) {
        triangles[i].setColor(color);
        rectangles[i].setColor(color);
    }
}

Vector<STANDARD_SIZE> Slope::getCenterOfSymmetry() {
    return centerOfSymmetry;
}
