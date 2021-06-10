#include <chrono>
#include <thread>
#include "Scene.h"

const std::string RED = "#FF0000";
const std::string BLACK = "#000000";
const std::string BLUE = "#0000FF";

bool gameLoop = true;

bool compareWithEpsilon(double a, double b, double epsilon);

Scene::Scene(double scale) {
    droneNumber = 0;
    drons.push_back(Drone(scale));
    drons.push_back(Drone(scale, Vector({200, 0, 0})));
    drons.push_back(Drone(scale, Vector({-200, 0, 0})));

    drons[0].setColor(RED);
    drons[1].setColor(BLUE);
    drons[2].setColor(BLACK);
}

void Scene::draw() {
    using clock = std::chrono::steady_clock;
    auto next_frame = clock::now();
    gameLoop = true;
    setPath();

    while (gameLoop) {
        next_frame += std::chrono::milliseconds(1000 / 60); // 60FPS

        if (commands.empty()) {
            clearPath();
            gameLoop = false;
        } else {
            droneCommandCenter();
        }

        for (int i = 0; i < obstacles.size(); ++i) {
            obstacles[i]->draw();
        }

        for (int i = 0; i < drons.size(); ++i) {
            drons[i].draw();
        }

        for (int i = 0; i < AMOUNT_OF_LINES; ++i) {
            lines[i].draw();
        }

        plot.initializePlot();

        std::this_thread::sleep_until(next_frame);
    }
}

void Scene::activateDrone() {
    int number = 0;
    for (int i = 0; i < drons.size(); ++i) {
        std::cout << "[" << i << "]" << " Drone: x= " << drons[i].getCenterOfSymmetry()[0]
        << " , z:" << drons[i].getCenterOfSymmetry()[2];
        if (i == droneNumber) {
            std::cout << " <---- activated \n";
        } else {
            std::cout << "\n";
        }
    }
    std::cout << "Enter number: \n";
    std::cin >> number;
    if (number < 0 || number >= drons.size()) {
        std::cout << "[Drones] Index out of range ! \n";
        return;
    }
    droneNumber = number;
}

void Scene::startDrone() {
    if (drons[droneNumber].getCenterOfSymmetry()[1] < 2 || commands.empty()) {
        commands.push_back(Command({'S', ALTITUDE}));
    } else {
        std::cout << "[Drone] Drone already lift  of ! \n";
    }
}

void Scene::landDrone() {
    if (drons[droneNumber].getCenterOfSymmetry()[1] > 2 || !commands.empty()) {
        commands.push_back(Command({'L', 0}));
    } else {
        std::cout << "[Drone] You can't land right now! \n";
    }
}

void Scene::rotateDrone() {
    double angle = 0;
    std::cout << "[Drone] Enter angle of rotation: ";
    std::cin >> angle;
    if (drons[droneNumber].getCenterOfSymmetry()[1] > 2 || !commands.empty()) {
        commands.push_back(Command({'R', angle}));
    } else {
        std::cout << "[Drone] You can't rotate right now! \n";
    }
}

void Scene::flyForwardDrone() {
    double distance = 0;
    std::cout << "[Drone] Enter distance: ";
    std::cin >> distance;
    if (drons[droneNumber].getCenterOfSymmetry()[1] > 2 || !commands.empty() || distance > 0) {
        commands.push_back(Command({'F', distance}));
    } else {
        std::cout << "[Drone] You can't flying right now! \n";
    }
}

void Scene::droneCommandCenter() {
    switch (commands[0].directing) {
        case 'S':
            drons[droneNumber].liftOf(commands[0].value);
            if (drons[droneNumber].getCenterOfSymmetry()[1] >= commands[0].value) {
                commands.erase(commands.begin());
            }
            break;
        case 'F':
            drons[droneNumber].forward();
            if (isTranslationDone()) {
                drons[droneNumber].setTranslationTotal(Vector());
                commands.erase(commands.begin());
            }
            break;
        case 'L':
            drons[droneNumber].landing();
            if (drons[droneNumber].getCenterOfSymmetry()[1] <= 1) {
                commands.erase(commands.begin());
            }
            break;
        case 'R':
            drons[droneNumber].rotation(commands[0].value);
            if (drons[droneNumber].getRotationVelocity() == 0 && isRotationDone()){
                commands.erase(commands.begin());
                Vector tmp = drons[droneNumber].getOrientationTotal();
                tmp = tmp + Vector({0, drons[droneNumber].getOrientationTemporary(), 0});
                drons[droneNumber].setOrientationTemporary(0);
                drons[droneNumber].setOrientationTotal(tmp);
            }
            break;
    }
}

bool Scene::isTranslationDone() {
    Vector tmp = calculateDistance(commands[0].value, drons[droneNumber].getOrientationTotal());
    bool x = compareWithEpsilon(drons[droneNumber].getTranslationTotal()[0], tmp[0], 6);
    bool y = compareWithEpsilon(drons[droneNumber].getTranslationTotal()[1], tmp[1], 6);
    bool z = compareWithEpsilon(drons[droneNumber].getTranslationTotal()[2], tmp[2], 6);
    return x && y && z;
}

bool Scene::isRotationDone() {
    return compareWithEpsilon(drons[droneNumber].getOrientationTemporary(), commands[0].value, 2);
}

void Scene::setPath() {
    Vector orientation({0, commands[1].value + drons[droneNumber].getOrientationTotal()[1], 0});
    Vector start = drons[droneNumber].getCenterOfSymmetry();
    start = start + Vector({0, ALTITUDE, 0});
    lines[0] = Polygon<LINE>({drons[droneNumber].getCenterOfSymmetry(), start});
    lines[0].setColor(drons[droneNumber].getColor());

    Vector land = calculateDistance(commands[2].value, orientation);
    land = land + drons[droneNumber].getCenterOfSymmetry();
    Vector tmp2 = land + Vector({0, ALTITUDE, 0});
    lines[2] = Polygon<LINE>({land, tmp2});
    lines[2].setColor(drons[droneNumber].getColor());

    lines[1] = Polygon<LINE>({start, tmp2});
    lines[1].setColor(drons[droneNumber].getColor());

}

void Scene::clearPath() {
    for (int i = 0; i < AMOUNT_OF_LINES; ++i) {
        lines[i].translation(10000000, 1000000, 0);
    }
}

void Scene::addObstacle() {
    int number = -1;
    double lenght = 0;
    double height = 0;
    double width = 0;
    double x = 0;
    double z = 0;

    std::cout << "Chose obstacle: \n";
    std::cout << "1 - Pyramid\n";
    std::cout << "2 - Slope\n";
    std::cout << "3 - Plateau\n";

    std::cin >> number;

    std::cout << "Enter lenght: \n";
    std::cin >> lenght;

    std::cout << "Enter height \n";
    std::cin >> height;

    std::cout << "Enter width: \n";
    std::cin >> width;

    std::cout << "Enter Cordinates x: \n";
    std::cin >> x;

    std::cout << "Enter Cordinates z: \n";
    std::cin >> z;

    switch (number) {
        case 1:
            obstacles.push_back(new Pyramid(lenght, height));
            break;
        case 2:
            obstacles.push_back(new Slope(lenght, height, width));
            break;
        case 3:
            obstacles.push_back(new Cuboid(width, height, lenght));
            obstacles[obstacles.size() - 1]->translation(Vector({0, height/2, 0}));
            break;
    }
    obstacles[obstacles.size() - 1]->translation(Vector({x, 0, z}));

}

void Scene::deleteObstacle() {
    if (obstacles.size() == 0) return;
    int id = -1;
    for (int i = 0; i < obstacles.size(); ++i) {
        std::cout << "[" << i << "] " << typeid(*obstacles[i]).name() + 1 << " - cordinates: x: "
        << obstacles[i]->getCenterOfSymmetry()[1] << ", y: " << obstacles[i]->getCenterOfSymmetry()[2] << "\n";

    }
    std::cout << "Enter obstacle id to delete: \n";
    std::cin >> id;
    if (id >= obstacles.size() || id < 0) return;
    obstacles[id]->translation(Vector({100000, 1000000, 0}));
    obstacles.erase(obstacles.begin() + id);
}

bool compareWithEpsilon(double a, double b, double epsilon) {
    return std::abs(a - b) < epsilon;
}

