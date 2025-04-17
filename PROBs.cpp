#define SFML_NO_AUDIO
#include <SFML/Graphics.hpp>
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <random>
#include <sstream>
#include <cstdlib>

using namespace std;
using namespace sf;
random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0, 1);
std::uniform_int_distribution<> child_dist(0, 1); // 50% chance
std::uniform_int_distribution<> role_dist(0, 3);
// ��������� ��� ����
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
const int INFO_AREA_HEIGHT = 100;
const int MAX_AGE = 10;
const int MAX_STOREROOM = 100;
const int MAX_ANTHILL_SIZE = 50;

// ������������ ����� ��������
enum class RoleType {
    None, Child, Nanny, Soldier, Gatherer, Builder, Shepherd
};

// ��������������� ���������� �������
class Formicidae;
class Role;
class AntHill;
class Enemy;

// ������� ����� ��� ����� ��������
class Role {
public:
    virtual ~Role() {}
    virtual void Work(Formicidae& ant) = 0;
    virtual string name() const = 0;
    virtual Color getColor() const = 0;
};

// ���������� ���������� �����
class Nanny : public Role {
public:
    void Work(Formicidae& ant) override {}
    string name() const override { return "Nanny"; }
    Color getColor() const override { return Color(255, 192, 203); } // �������
};

class Child : public Role {
public:
    void Work(Formicidae& ant) override {}
    string name() const override { return "Child"; }
    Color getColor() const override { return Color(173, 216, 230); } // �������
};

class Soldier : public Role {
public:
    void Work(Formicidae& ant) override {}
    string name() const override { return "Soldier"; }
    Color getColor() const override { return Color::Red; }
};

class Builder : public Role {
public:
    void Work(Formicidae& ant) override {}
    string name() const override { return "Builder"; }
    Color getColor() const override { return Color::Blue; }
};

class Shepherd : public Role {
public:
    void Work(Formicidae& ant) override {}
    string name() const override { return "Shepherd"; }
    Color getColor() const override { return Color::Green; }
};

// ����� �������
class Formicidae {
public:
    int age = 0;
    int health = 100;
    RoleType currentRoleType = RoleType::None;
    shared_ptr<Role> role;
    bool alive = true;
    bool grownUp = false;
    Vector2f position;
    Vector2f target;
    bool carryingItem = false;
    float speed = 1.0f + (rand() % 3);
    void ageUp() {
        age++;
        if (age >= 4 && !grownUp) {
            grownUp = true;
            // ��� ���������� ��������� ���� ������� ��� Builder ��� ������ �� 33%
            double r = dis(gen);
            if (r < 0.33) {
                role = make_shared<Soldier>();
                currentRoleType = RoleType::Soldier;
            }
            else if (r >= 0.33 && r < 0.66) {
                role = make_shared<Builder>();
                currentRoleType = RoleType::Builder;
            }
            else if (r >= 0.66) {
                role = make_shared<Nanny>();
                currentRoleType = RoleType::Nanny;
            }
        }
        if (age >= 19) {
            alive = false; // ������� ������� �� �����������
            return; // �������� ������� �� ������
        }
        else {
            updateRole();
        }
    }

    void updateRole() {
        if (age < 4) {
            role = make_shared<Child>(); currentRoleType = RoleType::Child;
        }
        else if (age >= 13) {
            role = make_shared<Shepherd>(); currentRoleType = RoleType::Shepherd;
        }
    }

    void work() {
        if (role && alive) role->Work(*this);
    }

    void receiveDamage(int damage) {
        health -= damage;
        if (health <= 0) {
            alive = false;
        }
    }
    Formicidae(Vector2f pos) : position(pos), target(pos) {}

    void update() {
        // �������� � ����
        Vector2f direction = target - position;
        float length = sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 1.0f) {
            direction /= length;
            position += direction * speed;
        }
    }

    void setTarget(Vector2f newTarget) {
        target = newTarget;
    }
};

// ����� �����
class Enemy {
public:
    Vector2f position;
    Vector2f target;
    int strength = 50;
    float speed = 1.5f;
    bool alive = true;

    Enemy(Vector2f pos) : position(pos), target(pos) {}

    void update() {
        Vector2f direction = target - position;
        float length = sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length > 1.0f) {
            direction /= length;
            position += direction * speed;
        }
    }
};

// ����� ����������� � �������������
class AntHill {
private:
    RenderWindow& window;
    Font font;
    RectangleShape infoArea;

    // ������� �������
    RectangleShape anthillShape;
    RectangleShape farmlandShape;
    RectangleShape forestShape;
    vector<RectangleShape> branches;
    vector<Formicidae> ants;
    vector<Enemy> enemies;

    // �������
    int day = 1;
    int count=0;
    int berries = 100;
    int branchesCount = 0;
    int anthillLevel = 1;
    int maxAnts = 50;

    // ��������� ��������� �����
    random_device rd;
    mt19937 gen;
    uniform_real_distribution<> dis;

public:
    AntHill(RenderWindow& win) : window(win), gen(rd()), dis(0, 1) {
        if (!font.openFromFile("arial.ttf")) {
            cerr << "Failed to load font!" << endl;
        }

        // ��������� ������� ��������
        infoArea.setSize(Vector2f(WINDOW_WIDTH, INFO_AREA_HEIGHT));
        infoArea.setPosition({ 0, WINDOW_HEIGHT - INFO_AREA_HEIGHT });
        infoArea.setFillColor(Color(200, 200, 200));

        anthillShape.setSize(Vector2f(200, 200));
        anthillShape.setPosition({ 50, 300 });
        anthillShape.setFillColor(Color(150, 75, 0));

        farmlandShape.setSize(Vector2f(300, 100));
        farmlandShape.setPosition({ 50, 50 });
        farmlandShape.setFillColor(Color(139, 69, 19));

        forestShape.setSize(Vector2f(300, 200));
        forestShape.setPosition({ WINDOW_WIDTH - 350, 50 });
        forestShape.setFillColor(Color(34, 139, 34));

        // �������� �����
        for (int i = 0; i < 20; ++i) {
            RectangleShape branch(Vector2f(20, 5));
            int xpos = 600 + rand() % 400;
            int ypos = 500 + rand() % 200;
            branch.setPosition(Vector2f(xpos, ypos));
            branch.setFillColor(Color(139, 69, 19));
            branch.setRotation(sf::degrees(rand() % 360));
            branches.push_back(branch);
        }

        // �������� ��������� ��������
        createInitialAnts();
    }

    void createInitialAnts() {
        Vector2f anthillCenter = anthillShape.getPosition() + Vector2f(anthillShape.getSize().x / 2, anthillShape.getSize().y / 2);

        // ������� ��������� �������� ������ �����
        for (int i = 0; i < 10; ++i) {
            Formicidae ant(anthillCenter);
            ant.role = make_shared<Child>();
            ant.currentRoleType = RoleType::Child;
            ants.push_back(ant);
        }
    }

    void update() {
        // ���������� ��������
        for (auto& ant : ants) {
            ant.update();

            // ������ ��������� � ����������� �� ����
            if (ant.role) {
                switch (ant.currentRoleType) {
                case RoleType::Soldier:
                    // ������� ����������� ������ �����������
                    if ((rand() % 100) < 2) {
                        Vector2f patrolPoint = anthillShape.getPosition() +
                            Vector2f(rand() % static_cast<int>(anthillShape.getSize().x),
                                rand() % static_cast<int>(anthillShape.getSize().y));
                        ant.setTarget(patrolPoint);
                    }
                    break;

                case RoleType::Builder:
                    // ��������� �������� �����
                    if (!ant.carryingItem) {
                        if (!branches.empty()) {
                            
                            (time(NULL));
                            int idx = rand() % branches.size();
                            ant.setTarget(branches[idx].getPosition());
                            ant.carryingItem = true;
                        }
                    }
                    else if (ant.carryingItem) {
                        ant.setTarget(anthillShape.getPosition() +
                            Vector2f(anthillShape.getSize().x / 2, anthillShape.getSize().y / 2));
                        if (Vector2f(ant.position - ant.target).length() < 10.0f) {
                            ant.carryingItem = false;
                            branchesCount++;
                        }
                    }
                    break;

                case RoleType::Shepherd:
                    // ������� ����� ����� ������������ � ������ �������� � ���� �����
                    break;
                }
            }
        }

        // ���������� ������
        for (auto& enemy : enemies) {
            enemy.update();
        }
        // ��������� ��������� ������
        if ((rand() % 1000) < 2 && day>10) {
            spawnEnemy();
        }
    }

    void spawnEnemy() {
        Vector2f spawnPoint(forestShape.getPosition().x + forestShape.getSize().x / 2,
            forestShape.getPosition().y + forestShape.getSize().y / 2);
        Enemy enemy(spawnPoint);
        enemy.target = anthillShape.getPosition() + Vector2f(anthillShape.getSize().x / 2, anthillShape.getSize().y / 2);
        enemies.push_back(enemy);
    }

    void draw() {
        // ��������� ������� ��������
        window.draw(anthillShape);
        window.draw(farmlandShape);
        window.draw(forestShape);

        // ��������� �����
        for (auto& branch : branches) {
            window.draw(branch);
        }

        // ��������� ��������
        for (auto& ant : ants) {
            CircleShape antShape(5);
            if (ant.role) {
                antShape.setFillColor(ant.role->getColor());
            }
            else {
                antShape.setFillColor(Color::White);
            }
            antShape.setPosition(ant.position);
            window.draw(antShape);

            // ���� ������� ����� �������
            if (ant.carryingItem) {
                RectangleShape item(Vector2f(3, 3));
                item.setFillColor(Color(139, 69, 19));
                item.setPosition(ant.position + Vector2f(5, 5));
                window.draw(item);
            }
        }

        // ��������� ������
        for (auto& enemy : enemies) {
            CircleShape enemyShape(8);
            enemyShape.setFillColor(Color::Black);
            enemyShape.setPosition(enemy.position);
            window.draw(enemyShape);
        }

        // ��������� �������������� ������
        window.draw(infoArea);
        drawInfoText();
        count++;
        if (count == 200) {
            day++;
            for (auto ant : ants) {
                ant.ageUp();
            }
            count = 0;
        }
    }

    void drawInfoText() {
        Text infoText(font);
        infoText.setCharacterSize(16);
        infoText.setFillColor(Color::Black);

        stringstream ss;
        ss << "Day: " << day << " | Berries: " << berries << " | Branches: " << branchesCount
            << " | Ants: " << ants.size() << "/" << maxAnts << " | Level: " << anthillLevel;
        infoText.setString(ss.str());
        infoText.setPosition({ 10, WINDOW_HEIGHT - INFO_AREA_HEIGHT + 10 });
        window.draw(infoText);

        // ������� �������� �� �����
        int soldiers = 0, builders = 0, shepherds = 0, nannies = 0, children = 0;
        for (auto& ant : ants) {
            if (ant.role) {
                if (ant.currentRoleType == RoleType::Soldier) soldiers++;
                else if (ant.currentRoleType == RoleType::Builder) builders++;
                else if (ant.currentRoleType == RoleType::Shepherd) shepherds++;
                else if (ant.currentRoleType == RoleType::Nanny) nannies++;
                else if (ant.currentRoleType == RoleType::Child) children++;
            }
        }

        ss.str("");
        ss << "Roles: Soldiers: " << soldiers << " | Builders: " << builders
            << " | Shepherds: " << shepherds << " | Nannies: " << nannies
            << " | Children: " << children;
        infoText.setString(ss.str());
        infoText.setPosition({ 10, WINDOW_HEIGHT - INFO_AREA_HEIGHT + 35 });
        window.draw(infoText);
    }
};

int main() {
    RenderWindow window(VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "Ant Colony Simulation");
    window.setFramerateLimit(60);

    AntHill anthill(window);
    while (window.isOpen()) {
        while (const optional event = window.pollEvent()) {
            if (event->is<Event::Closed>())
                window.close();
        }

        window.clear(Color::White);

        anthill.update();
        anthill.draw();

        window.display();
    }

    return 0;
}