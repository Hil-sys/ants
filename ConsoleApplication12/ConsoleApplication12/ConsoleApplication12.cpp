﻿#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <random>

using namespace std; // добавляем для устранения необходимости писать std::

random_device rd;
mt19937 gen(rd());
uniform_real_distribution<> dis(0, 1);
std::uniform_int_distribution<> child_dist(0, 1); // 50% chance
std::uniform_int_distribution<> role_dist(0, 3);  // 4 роли

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
const int MAX_ANTHILL_LEVEL = 12;
const int MAX_ANTHILL_SIZE = 50;

// Перечисление ролей
enum class RoleType {
    None, Child, Nanny, Soldier, Gatherer, Builder, Shepherd
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

class Child : public Role {
public:
    void Work(Formicidae& ant) override {
        cout << "Дети резвятся в муравейнике\n";
    }
    string name() const override { return "Ребенок"; }
};

class Soldier : public Role {
public:
    void Work(Formicidae& ant) override {
        cout << "Солдат защищает муравейник\n";
    }
    string name() const override { return "Солдат"; }
};

//class Gatherer : public Role {
//public:
//    void Work(Formicidae& ant) override {
//        cout << "Собиратель собирает еду\n";
//    }
//    string name() const override { return "Собиратель"; }
//};

class Builder : public Role {
public:
    void Work(Formicidae& ant) override {
        cout << "Строитель расширяет муравейник и собирает палочки\n";
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
            // При взрослении назначаем роль солдата или Builder или нянька по 33%
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
            alive = false; // убираем муравья из муравейника
            return; // досрочно выходим из метода
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
    int food = 100;
    int maxFood = MAX_STOREROOM;
    int maxAnts = 50;
    int branches = 0;
    int level = 1;
    int maxBranches = 300; // произвольный максимум веточек


    vector<shared_ptr<Formicidae>> ants;
    vector<Observer*> observers;

    void addAnt(shared_ptr<Formicidae> ant) {
        if (ants.size() < maxAnts && size >= static_cast<int>(ants.size()) + 1 && food >= 5) {
            addFood(-5);
            ants.push_back(ant);
            cout << "У нас пополнение!\n"; 
        }
    }
    void removeDeadAnts() {
        ants.erase(std::remove_if(ants.begin(), ants.end(),
            [](const shared_ptr<Formicidae>& ant) {
                if (ant->health <= 0) {
                    cout << "Сегодня от голода умер " << (ant->role ? ant->role->name() : "без роли") << "\n";
                    return true;
                }
                else if (!ant->alive) {
                    cout << "Сегодня от старости умер " << (ant->role ? ant->role->name() : "без роли") << "\n";
                    return true;
                }
                return false;
            }),
            ants.end());
    }

    void addBranches(int amount) {
        branches += amount;
        if (branches > maxBranches) branches = maxBranches;
        if (branches < 0) branches = 0;
    }
    void firstDay() {
        // Инициализация генератора случайных чисел
        auto newAnt = make_shared<Formicidae>();
        newAnt->role = make_shared<Nanny>();
        newAnt->currentRoleType = RoleType::Nanny;
        newAnt->age = 4;
        newAnt->grownUp = true;



        addAnt(newAnt);

        for (int i = 0; i < (maxAnts / 2) - 1; ++i) {
            auto newAnt = make_shared<Formicidae>();

            // 50% chance для ребенка, 50% для случайной роли
            if (child_dist(gen)) {
                newAnt->role = make_shared<Child>();
                newAnt->currentRoleType = RoleType::Child;
            }
            else {
                // Генерация случайной роли для взрослого муравья
                switch (role_dist(gen)) {
                case 0:
                    newAnt->role = make_shared<Nanny>();
                        newAnt->currentRoleType = RoleType::Nanny;
                        newAnt->age = 4;
                        newAnt->grownUp = true;
                    break;
                case 1:
                    newAnt->role = make_shared<Soldier>();
                    newAnt->currentRoleType = RoleType::Soldier;
                    newAnt->age = 4;
                    newAnt->grownUp = true;
                    break;
                case 2:
                    newAnt->role = make_shared<Builder>();
                    newAnt->currentRoleType = RoleType::Builder;
                    newAnt->age = 4;
                    newAnt->grownUp = true;
                    break;
                case 3:
                    newAnt->role = make_shared<Shepherd>();
                    newAnt->currentRoleType = RoleType::Shepherd;
                    newAnt->age = 13; // Пастухи старше
                    newAnt->grownUp = true;
                    break;
                }
            }
            addAnt(newAnt);
        }
    }
    void simulateDay() {
        for (auto& ant : ants) {
            if (ant->alive) {
                if (ant->currentRoleType == RoleType::Shepherd) {
                    addFood(8);
                }
                if (ant->currentRoleType == RoleType::Nanny) {
                    addFood(5);
                }
                if (ant->currentRoleType == RoleType::Builder) {
                    addBranches(1); // сбор одного веточки
                }
                if (branches >= 3) {
                    size += 1; // увеличение муравейника
                    if (size > maxSize) size = maxSize;
                    branches -= 3; // расход веточек
                }
                ant->work();
                ant->ageUp();
            }
            
        }
        
        removeDeadAnts();
        /*ants.erase(remove_if(ants.begin(), ants.end(),
            [](const shared_ptr<Formicidae>& a) { return !a->alive; }),
            ants.end());*/

        // Обновление размера муравейника при достаточном уровне еды
        /*if (food > maxFood * 0.5 && size < maxSize) {
            size += 1;
            maxAnts = size;
        }*/
        

        // Пример добавления нового муравья-няни в начале дня
        auto newAnt = make_shared<Formicidae>();
        newAnt->role = make_shared<Child>();
        newAnt->currentRoleType = RoleType::Child;

        

        addAnt(newAnt); // добавляем в муравейник с проверками

        // Регулировка еды
        /*if (food < maxFood) {
            food += rand() % 10;
            if (food > maxFood) food = maxFood;
        }*/
        
        for (auto& ant : ants) {
            if (ant->alive) {
                if (ant->currentRoleType == RoleType::Child) {
                    removeFood(2);
                }
                else if (ant->currentRoleType == RoleType::Nanny) {
                    removeFood(1);
                }
                else if (ant->currentRoleType == RoleType::Soldier) {
                    removeFood(3);
                }
                else if (ant->currentRoleType == RoleType::Builder) {
                    removeFood(3);
                }
                else if (ant->currentRoleType == RoleType::Shepherd) {
                    removeFood(2);
                }
            }
        }
        if (food == 0) {
            cout << "Муравьи голодают!\n";
            for (auto& ant : ants) {
                if (ant->alive) {
                    if (ant->currentRoleType == RoleType::Child) {
                        ant->health -= 50;
                    }
                    else ant->health -= 25;
                }
            }
        }
    }


    void addFood(int amount) {
        if (food + amount <= maxFood)
            food += amount;
        else
            food = maxFood;
    }

    void removeFood(int amount) {
        if (food - amount >= 0)
            food -= amount;
        else
            food = 0;
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
        if (hill.ants.empty()) return;

        // Случайный выбор жертв
        random_shuffle(hill.ants.begin(), hill.ants.end());

        int countToKill = min<int>(1 + rand() % 3, hill.ants.size());

        // Перед удалением собираем информацию о ролях
        vector<string> killedRoles;
        for (int i = 0; i < countToKill; ++i) {
            if (hill.ants[i]->role) {
                killedRoles.push_back(hill.ants[i]->role->name());
            }
            else {
                killedRoles.push_back("Без роли");
            }
        }

        // Удаляем муравьев
        hill.ants.erase(hill.ants.begin(), hill.ants.begin() + countToKill);

        // Выводим информацию об убитых
        cout << "Убито муравьев: " << countToKill << endl;
        cout << "Роли убитых: ";
        for (const auto& role : killedRoles) {
            cout << role << ", ";
        }
        cout << endl;

        // Гарантированная кража
        int foodStolen = min(1 + rand() % 25, hill.food);
        hill.food -= foodStolen;

        hill.notify("Враг украл " + to_string(foodStolen) + " еды!");
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

    


    for (int day = 0; day < 26; ++day) {
        cout << "День " << (day) << "\n";

        if (day == 9) {
            Enemy enemy;
            enemy.attack(hill);
        }
        if (day == 0)
            hill.firstDay();
        else
            hill.simulateDay();

        cout << "Статистика:\n";
        cout << "Размер муравейника: " << hill.size << "\n";
        cout << "Количество муравьев: " << hill.ants.size() << "\n";
        cout << "Веточки: " << hill.branches << "\n";
        cout << "Количество еды: " << hill.food << "\n";

        int countChild = 0, countNanny = 0, countSoldier = 0, countGatherer = 0, countBuilder = 0, countShepherd = 0;
        for (auto& ant : hill.ants) {
            switch (ant->currentRoleType) {
            case RoleType::Nanny: countNanny++; break;
            case RoleType::Child: countChild++; break;
            case RoleType::Soldier: countSoldier++; break;
            case RoleType::Gatherer: countGatherer++; break;
            case RoleType::Builder: countBuilder++; break;
            case RoleType::Shepherd: countShepherd++; break;
            default: break;
            }
        }

        cout << "Роли:\n";
        cout << "\tДети: " << countChild << "\n";
        cout << "\tНяни: " << countNanny << "\n";
        cout << "\tСолдаты: " << countSoldier << "\n";
        cout << "\tСтроители: " << countBuilder << "\n";
        cout << "\tПастухи: " << countShepherd << "\n";

        cout << "------------------------\n";
    }

    return 0;
}
