#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>

class Button {
public:
    Button(float x, float y, const std::string& text, sf::Font& font) {
        button.setSize(sf::Vector2f(300, 50));
        button.setPosition(x, y);
        button.setFillColor(sf::Color::Blue);

        buttonText.setFont(font);
        buttonText.setString(text);
        buttonText.setCharacterSize(24);
        buttonText.setFillColor(sf::Color::White);
        buttonText.setPosition(x + 20, y + 10);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(button);
        window.draw(buttonText);
    }

private:
    sf::RectangleShape button;
    sf::Text buttonText;
};

class MazeGame {
public:
    MazeGame()
        : window(sf::VideoMode(800, 600), "Mitchell's Maze"), score(0), playerSpeed(5.f), gameState(Menu),
        startButton(250, 250, "Press 1 to START", font), exitButton(250, 320, "Press 2 to EXIT", font),
        backToMenuButton(10, 10, "Press 2 to RETURN", font)
    {
        if (!font.loadFromFile("arial.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
            exit(1);
        }

        // Player Sprite.
        player.setSize(sf::Vector2f(20, 20));
        player.setFillColor(sf::Color::Blue);
        player.setPosition(100, 100);

        // Generate the randomized Maze.
        std::srand(std::time(nullptr));
        generateNewMaze();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            draw();
        }
    }

private:
    enum State { Menu, Game };
    sf::RenderWindow window;
    sf::Font font;
    Button startButton;
    Button exitButton;
    Button backToMenuButton;
    sf::RectangleShape player;
    int score;
    float playerSpeed;
    State gameState;
    sf::RectangleShape startZone;
    sf::RectangleShape endZone;
    std::vector<sf::RectangleShape> walls;

    void update() {
        if (gameState == Game) {
            if (checkCollision(player)) {
                // Reached the end, reset to entrance.
                player.setPosition(startZone.getPosition());
                generateNewMaze();
                score += 100;
            }
        }
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (gameState == Menu) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Num1) {
                        gameState = Game;
                        generateNewMaze();
                        player.setPosition(startZone.getPosition()); // Ensure player starts at Entrance
                    }
                    if (event.key.code == sf::Keyboard::Num2) {
                        window.close();  // This Num2 closes the game.
                    }
                }
            }

            if (gameState == Game) {
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Up) {
                        movePlayer(0, -playerSpeed);
                    }
                    else if (event.key.code == sf::Keyboard::Down) {
                        movePlayer(0, playerSpeed);
                    }
                    else if (event.key.code == sf::Keyboard::Left) {
                        movePlayer(-playerSpeed, 0);
                    }
                    else if (event.key.code == sf::Keyboard::Right) {
                        movePlayer(playerSpeed, 0);
                    }
                    else if (event.key.code == sf::Keyboard::Num2) {
                        gameState = Menu;  // This Num2 returns user back to the Menu.
                    }
                }
            }
        }
    }

    void draw() {
        window.clear();

        if (gameState == Menu) {
            drawMenu();
        }
        else if (gameState == Game) {
            drawGame();
        }

        window.display();
    }

    void drawMenu() {
        // Draw Title.
        sf::Text title("Mitchell's Maze", font, 40);
        title.setFillColor(sf::Color::White);
        title.setPosition(250, 100);
        window.draw(title);

        // Draw Buttons for the Menu.
        startButton.draw(window);
        exitButton.draw(window);
    }

    void drawGame() {
        // Draw Maze and player.
        window.draw(startZone);
        window.draw(endZone);
        for (auto& wall : walls) {
            window.draw(wall);
        }

        window.draw(player);

        // Draw Score.
        sf::Text scoreText("Score: " + std::to_string(score), font, 24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(650, 20);
        window.draw(scoreText);

        // Draw Return Button.
        backToMenuButton.draw(window);
    }

    void movePlayer(float dx, float dy) {
        sf::Vector2f newPos = player.getPosition() + sf::Vector2f(dx, dy);

        // Check for collision with walls before moving the player.
        if (!isCollidingWithWalls(newPos)) {
            player.setPosition(newPos);
        }
    }

    bool checkCollision(sf::RectangleShape& player) {
        // Check if the player reaches the end zone.
        if (player.getGlobalBounds().intersects(endZone.getGlobalBounds())) {
            return true;
        }
        return false;
    }

    bool isCollidingWithWalls(const sf::Vector2f& newPos) {
        // Check if the player's new position collides with any walls.
        for (auto& wall : walls) {
            if (wall.getGlobalBounds().intersects(sf::FloatRect(newPos, player.getSize()))) {
                return true;
            }
        }
        return false;
    }

    void generateNewMaze() {
        walls.clear();

        // Maze grid size.
        const int rows = 14;
        const int cols = 14;
        const float tileSize = 30.f;

        // Create a 2D array to represent the maze
        std::vector<std::vector<int>> maze(rows, std::vector<int>(cols, 1)); // 1 = wall, 0 = path.

        // Set the outer boundary walls (top, bottom, left, and right).
        for (int x = 0; x < cols; ++x) {
            maze[0][x] = 1;  // Top row.
            maze[rows - 1][x] = 1;  // Bottom row.
        }
        for (int y = 0; y < rows; ++y) {
            maze[y][0] = 1;  // Left column.
            maze[y][cols - 1] = 1;  // Right column.
        }
        
        // Set Entrance and exit positions (inside the boundaries).
        int startX = 1, startY = 1;  // Starting position inside the boundary (top left corner).
        int endX = cols - 2, endY = rows - 3;  // Ending position (bottom right corner).

        // Mark Entrance and exit positions (make them paths, not walls).
        maze[startY][startX] = 0;
        maze[endY][endX] = 0;

        // Recursive backtracking algorithm for maze generation.
        std::vector<std::pair<int, int>> stack;
        stack.push_back({ startX, startY });
        maze[startY][startX] = 0;  // Start point as a path.

        while (!stack.empty()) {
            auto current = stack.back();
            stack.pop_back();

            int x = current.first;
            int y = current.second;

            // Randomly shuffle the directions to create a random maze.
            std::vector<std::pair<int, int>> directions = {
                {0, 2}, {2, 0}, {0, -2}, {-2, 0}
            };
            std::random_shuffle(directions.begin(), directions.end());

            for (const auto& dir : directions) {
                int nx = x + dir.first;
                int ny = y + dir.second;

                // Check bounds and if the cell is unvisited (make sure to not go outside the maze).
                if (nx > 0 && nx < cols - 1 && ny > 0 && ny < rows - 1 && maze[ny][nx] == 1) {
                    maze[ny][nx] = 0;  // Make the cell a path
                    maze[y + dir.second / 2][x + dir.first / 2] = 0;  // Carve the wall between cells.
                    stack.push_back({ nx, ny });
                }
            }
        }

        // Calculate the offset to center the maze in the window.
        const float windowWidth = window.getSize().x;
        const float windowHeight = window.getSize().y;
        const float mazeWidth = cols * tileSize;
        const float mazeHeight = rows * tileSize;

        const float offsetX = (windowWidth - mazeWidth) / 2;
        const float offsetY = (windowHeight - mazeHeight) / 2;

        // Set position of the maze based on window center.
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                if (maze[y][x] == 1) {
                    sf::RectangleShape wall(sf::Vector2f(tileSize, tileSize));
                    wall.setPosition(x * tileSize + offsetX, y * tileSize + offsetY);
                    wall.setFillColor(sf::Color::White);
                    walls.push_back(wall);
                }
            }
        }

        // Set position of Entrance and Exit based on window center.
        startZone.setSize(sf::Vector2f(tileSize, tileSize));
        startZone.setPosition(startX * tileSize + offsetX, startY * tileSize + offsetY);
        startZone.setFillColor(sf::Color::Green);

        endZone.setSize(sf::Vector2f(tileSize, tileSize));
        endZone.setPosition(endX * tileSize + offsetX, endY * tileSize + offsetY);
        endZone.setFillColor(sf::Color::Red);
    }
};

int main() {
    MazeGame game;
    game.run();
    return 0;
}