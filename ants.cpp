#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <random>

using namespace std; // добавляем для устранения необходимости писать std::

random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0, 1);

// Объявление предварительных классов
class Formicidae;
class Role;
class WorkObserver;
class AntHill;
class Enemy;

// Статические константы
const int MAX_AGE = 10;
const int MAX_ROLES = 4;
const int MAX_SOLDIER_ENEMY = 3;
const int MAX_STOREROOM = 100;
const int MAX_ANTHILL_SIZE = 50;

// Перечисление ролей
enum class RoleType { None, Nanny, Soldier, Gatherer, Builder, Shepherd
};

// ----------------------
// АБСТРАКТНЫЙ КЛАСС РОЛЬ
class Role {
public:
    virtual ~Role() {}
    virtual void Work(Formicidae& ant) = 0;
    virtual string name() const = 0;
};

// ----------------------
// Конкретные роли
class Nanny : public Role {
public:
    void Work(Formicidae& ant) override {
        cout << "Няня заботится о муравьях\n";
    }
    string name() const override { return "Няня"; }
};

class Soldier : public Role {
public:
    void Work(Formicidae& ant) override {
        cout << "Солдат защищает муравейник\n";
    }
    string name() const override { return "Солдат"; }
};

class Gatherer : public Role {
public:
    void Work(Formicidae& ant) override {
        cout << "Собиратель собирает еду\n";
    }
    string name() const override { return "Собиратель"; }
};

class Builder : public Role {
public:
    void Work(Formicidae& ant) override {
        cout << "Строитель расширяет муравейник\n";
    }
    string name() const override { return "Строитель"; }
};

class Shepherd : public Role {
public:
    void Work(Formicidae& ant) override {
        cout << "Пастух собирает еду\n";
    }
    string name() const override { return "Пастух"; }
};

// ----------------------
// МУРАВЕЙ
class Formicidae {
public:
    int age = 0;
    int health = 100;
    RoleType currentRoleType = RoleType::None;
    shared_ptr<Role> role;
    bool alive = true;
    bool grownUp = false; // добавляем флаг

    void ageUp() {
        age++;
        if (age >= 4 && !grownUp) {
            grownUp = true;
            // При взрослении назначаем роль солдата или сборщика по 50%
            double r = dis(gen);
            if (r < 0.5) {
                role = make_shared<Soldier>();
                currentRoleType = RoleType::Soldier;
            }
            else {
                role = make_shared<Gatherer>();
                currentRoleType = RoleType::Gatherer;
            }
        }
        if (age >= 17) {
            alive = false; // убираем муравья из муравейника
            return; // досрочно выходим из метода
        }
        else {
            updateRole();
        }
    }

    void updateRole() {
        if (age < 4) {
            role = make_shared<Nanny>(); currentRoleType = RoleType::Nanny;
        }
        else if (age > 12) {
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
};


// ---------------------
// ИНТЕРФЕЙС ОПОВЕЩЕНИЙ
class INotifier {
public:
    virtual ~INotifier() {}
    virtual void notify(const string& message) = 0;
};

class Observer {
public:
    virtual void update(const string& event) = 0;
};

// ----------------------
// Класс "Муравейник"
class AntHill : public INotifier {
public:
    int size = 10;
    int maxSize = MAX_ANTHILL_SIZE;
    int food = 50;
    int maxFood = MAX_STOREROOM;
    int maxAnts = 50;

    vector<shared_ptr<Formicidae>> ants;
    vector<Observer*> observers;

    void addAnt(shared_ptr<Formicidae> ant) {
        if (ants.size() < maxAnts && size >= static_cast<int>(ants.size()) + 1 && food >= 10) {
            addFood(-10);
            ants.push_back(ant);
        }
    }


    void simulateDay() {
        for (auto& ant : ants) {
            if (ant->alive) {
                if (ant->currentRoleType == RoleType::Shepherd) {
                    addFood(3);
                }
                ant->work();
                ant->ageUp();
            }
        }
        ants.erase(remove_if(ants.begin(), ants.end(),
            [](const shared_ptr<Formicidae>& a) { return !a->alive; }),
            ants.end());

        // Обновление размера муравейника при достаточном уровне еды
        /*if (food > maxFood * 0.5 && size < maxSize) {
            size += 1;
            maxAnts = size;
        }*/

        // Пример добавления нового муравья-няни в начале дня
        auto newAnt = make_shared<Formicidae>();
        newAnt->role = make_shared<Nanny>();
        newAnt->currentRoleType = RoleType::Nanny;

        addAnt(newAnt); // добавляем в муравейник с проверками

        // Регулировка еды
        if (food < maxFood) {
            food += rand() % 5;
            if (food > maxFood) food = maxFood;
        }
        else if (food > 0) {
            food -= rand() % 3;
            if (food < 0) food = 0;
        }
    }

    void addFood(int amount) {
        if (food + amount <= maxFood)
            food += amount;
        else
            food = maxFood;
    }

    void notify(const string& message) override {
        for (auto* obs : observers)
            obs->update(message);
    }

    void addObserver(Observer* obs) { observers.push_back(obs); }
    void removeObserver(Observer* obs) {
        observers.erase(remove(observers.begin(), observers.end(), obs), observers.end());
    }
};

// ----------------------
// Враг
class Enemy {
public:
    int strength = 50;

    void attack(AntHill& hill) {
        int countToKill = min<int>((int)hill.ants.size(), rand() % 3 + 1);
        hill.ants.erase(hill.ants.begin(), hill.ants.begin() + countToKill);
        hill.food -= rand() % 10;
        if (hill.food < 0) hill.food = 0;
        hill.notify("Враг напал на муравейник!");
    }
};

// ----------------------
// Класс "Информер" для оповещений
class EventManager : public Observer {
public:
    string name;
    EventManager(const string& n) : name(n) {}
    void update(const string& event) override {
        cout << "[" << name << "] Оповещение: " << event << "\n";
    }
};

// --------------------------
// Основная функция
int main() {
    setlocale(LC_ALL, "Russian");
    AntHill hill;

    // Создаем подписчиков
    EventManager em1("Информёр 1");
    EventManager em2("Информёр 2");
    hill.addObserver(&em1);
    hill.addObserver(&em2);

    for (int i = 0; i < 20; i++) {
        auto ant = make_shared<Formicidae>();
        // сразу задаем роль няньки
        ant->role = make_shared<Nanny>();
        ant->currentRoleType = RoleType::Nanny;
        hill.addAnt(ant);
    }


    for (int day = 0; day < 25; ++day) {
        cout << "День " << (day) << "\n";

        /*if (day == 9) {
            Enemy enemy;
            enemy.attack(hill);
        }*/

        hill.simulateDay();

        cout << "Статистика:\n";
        cout << "Размер муравейника: " << hill.size << "\n";
        cout << "Количество муравьев: " << hill.ants.size() << "\n";
        cout << "Количество еды: " << hill.food << "\n";

        int countNanny = 0, countSoldier = 0, countGatherer = 0, countBuilder = 0, countShepherd = 0;
        for (auto& ant : hill.ants) {
            switch (ant->currentRoleType) {
            case RoleType::Nanny: countNanny++; break;
            case RoleType::Soldier: countSoldier++; break;
            case RoleType::Gatherer: countGatherer++; break;
            case RoleType::Builder: countBuilder++; break;
            case RoleType::Shepherd: countShepherd++; break;
            default: break;
            }
        }

        cout << "Роли:\n";
        cout << "\tНяни: " << countNanny << "\n";
        cout << "\tСолдаты: " << countSoldier << "\n";
        cout << "\tСобиратели: " << countGatherer << "\n";
        cout << "\tСтроители: " << countBuilder << "\n";
        cout << "\tПастухи: " << countShepherd << "\n";

        cout << "------------------------\n";
    }

    return 0;
}
