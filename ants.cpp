#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <ctime>
#include <random>

using namespace std;

// Перечисление социальных ролей муравьев
enum class SocialRole {
    NONE,       // Нет роли (молодые муравьи)
    NURSE,      // Нянька
    SOLDIER,    // Солдат
    SHEPHERD,   // Пастух (при плохом здоровье)
    GATHERER,   // Собиратель
    BUILDER,    // Строитель
    CLEANER     // Уборщик
};

// Класс для оповещений
class Informer;

// Абстрактный класс Роль
class Role {
public:
    virtual ~Role() = default;
    virtual void work() = 0;
    virtual SocialRole getRoleType() const = 0;
    virtual void handleNotification(const string& message) = 0;
};

// Класс Муравей
class Ant {
private:
    int age;
    int health;
    SocialRole currentRole;
    unique_ptr<Role> role;
    shared_ptr<Informer> informer;

public:
    Ant(int initialAge = 0, int initialHealth = 100)
        : age(initialAge), health(initialHealth), currentRole(SocialRole::NONE) {}

    void update() {
        age++;
        updateRole();
        if (role) {
            role->work();
        }
        // Постепенное уменьшение здоровья с возрастом
        if (age > 30) {
            health -= 1;
        }
    }

    void updateRole() {
        SocialRole newRole = determineRole();
        if (newRole != currentRole) {
            currentRole = newRole;
            assignRole();
        }
    }

    SocialRole determineRole() const {
        if (age < 3) return SocialRole::NONE;
        if (age < 7) return SocialRole::NURSE;
        if (age < 15) {
            return health > 70 ? SocialRole::SOLDIER : SocialRole::SHEPHERD;
        }
        if (age < 30) {
            return health > 50 ? SocialRole::GATHERER : SocialRole::BUILDER;
        }
        return SocialRole::CLEANER;
    }

    void assignRole() {
        // Здесь должна быть реализация создания конкретной роли
        // В зависимости от currentRole
        // Например:
        // role = make_unique<NurseRole>(this);
    }

    void setHealth(int newHealth) { health = newHealth; }
    int getHealth() const { return health; }
    int getAge() const { return age; }
    SocialRole getCurrentRole() const { return currentRole; }

    void notify(const string& message) {
        if (role) {
            role->handleNotification(message);
        }
    }
};

// Конкретные роли (примеры)
class NurseRole : public Role {
private:
    Ant* ant;
public:
    NurseRole(Ant* a) : ant(a) {}
    void work() override {
        cout << "Нянька ухаживает за личинками" << endl;
        ant->setHealth(ant->getHealth() - 1); // Работа утомляет
    }
    SocialRole getRoleType() const override { return SocialRole::NURSE; }
    void handleNotification(const string& message) override {
        if (message == "attack") {
            cout << "Нянька спасает личинок!" << endl;
        }
    }
};

class SoldierRole : public Role {
private:
    Ant* ant;
public:
    SoldierRole(Ant* a) : ant(a) {}
    void work() override {
        cout << "Солдат патрулирует территорию" << endl;
        ant->setHealth(ant->getHealth() - 2); // Тяжелая работа
    }
    SocialRole getRoleType() const override { return SocialRole::SOLDIER; }
    void handleNotification(const string& message) override {
        if (message == "attack") {
            cout << "Солдат бежит на защиту!" << endl;
        }
    }
};

// Класс для оповещений
class Informer {
private:
    vector<Ant*> subscribers;
public:
    void subscribe(Ant* ant) {
        subscribers.push_back(ant);
    }
    void unsubscribe(Ant* ant) {
        subscribers.erase(remove(subscribers.begin(), subscribers.end(), ant), subscribers.end());
    }
    void notifyAll(const string& message) {
        for (auto& sub : subscribers) {
            sub->notify(message);
        }
    }
};

// Класс Враг
class Enemy {
private:
    int strength;
public:
    Enemy(int str) : strength(str) {}
    bool attack() {
        return (rand() % 100) < strength; // Шанс успешной атаки
    }
    void setStrength(int str) { strength = str; }
    int getStrength() const { return strength; }
};

// Класс Муравейник
class AntHill {
private:
    int size;
    int maxAnts;
    int currentAnts;
    int foodStorage;
    int buildingMaterials;
    vector<Ant> ants;
    vector<shared_ptr<Informer>> informers;
    vector<Enemy> enemies;

public:
    AntHill(int initialSize = 10)
        : size(initialSize), maxAnts(initialSize * 10), currentAnts(0),
        foodStorage(0), buildingMaterials(0) {
        initializeInformers();
    }

    void initializeInformers() {
        // Создаем информеры для разных типов событий
        informers.resize(7); // По количеству ролей + общие
    }

    void update() {
        // Обновляем всех муравьев
        for (auto& ant : ants) {
            ant.update();
        }

        // Проверяем условия для изменения муравейника
        if (buildingMaterials > size * 5) {
            expandHill();
        }
        else if (buildingMaterials == 0) {
            decayHill();
        }

        // Проверяем запасы еды
        if (foodStorage == 0) {
            starveAnts();
        }
        else if (foodStorage > maxAnts * 2) {
            // Избыток еды - можно размножаться
            reproduceAnts();
        }

        // Проверяем нападения врагов
        handleEnemyAttacks();
    }

    void expandHill() {
        size++;
        maxAnts = size * 10;
        buildingMaterials -= size * 5;
        cout << "Муравейник расширился! Новый размер: " << size << endl;
    }

    void decayHill() {
        size = max(1, size - 1);
        maxAnts = size * 10;
        cout << "Муравейник уменьшился! Новый размер: " << size << endl;
    }

    void starveAnts() {
        for (auto& ant : ants) {
            ant.setHealth(ant.getHealth() - 5);
        }
        // Удаляем мертвых муравьев
        ants.erase(remove_if(ants.begin(), ants.end(),
            [](const Ant& a) { return a.getHealth() <= 0; }), ants.end());
    }

    void reproduceAnts() {
        if (ants.size() >= maxAnts) return;

        int newAnts = min(5, maxAnts - static_cast<int>(ants.size()));
        for (int i = 0; i < newAnts; ++i) {
            ants.emplace_back(0, 100);
        }
        foodStorage -= newAnts * 2;
    }

    void handleEnemyAttacks() {
        if (enemies.empty() && (rand() % 100) < 5) {
            // С шансом 5% появляется новый враг
            enemies.emplace_back(rand() % 30 + 20);
        }

        for (auto& enemy : enemies) {
            if (enemy.attack()) {
                cout << "Враг атакует!" << endl;
                informers[static_cast<int>(SocialRole::SOLDIER)]->notifyAll("attack");

                // Простая логика боя
                int soldierCount = count_if(ants.begin(), ants.end(),
                    [](const Ant& a) { return a.getCurrentRole() == SocialRole::SOLDIER; });

                if (soldierCount > 0 && (rand() % 100) < (soldierCount * 5)) {
                    cout << "Солдаты победили врага!" << endl;
                    enemies.erase(remove(enemies.begin(), enemies.end(), enemy), enemies.end());
                }
                else {
                    cout << "Враг нанес урон!" << endl;
                    // Враг убивает случайного муравья
                    if (!ants.empty()) {
                        ants.erase(ants.begin() + rand() % ants.size());
                    }
                }
            }
        }
    }

    void addFood(int amount) {
        if (foodStorage + amount <= maxAnts * 3) {
            foodStorage += amount;
        }
    }

    void addBuildingMaterials(int amount) {
        buildingMaterials += amount;
    }

    int getSize() const { return size; }
    int getAntCount() const { return static_cast<int>(ants.size()); }
    int getFoodStorage() const { return foodStorage; }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    AntHill hill(5);

    // Добавляем начальных муравьев
    for (int i = 0; i < 20; ++i) {
        hill.addFood(10);
        hill.addBuildingMaterials(5);
    }

    // Симуляция жизни муравейника
    for (int day = 1; day <= 30; ++day) {
        cout << "\n=== День " << day << " ===" << endl;
        cout << "Размер муравейника: " << hill.getSize() << endl;
        cout << "Количество муравьев: " << hill.getAntCount() << endl;
        cout << "Запасы еды: " << hill.getFoodStorage() << endl;

        hill.update();

        // Собиратели и строители приносят ресурсы
        if (day % 3 == 0) {
            hill.addFood(rand() % 10 + 5);
            hill.addBuildingMaterials(rand() % 5 + 2);
        }
    }

    return 0;
}