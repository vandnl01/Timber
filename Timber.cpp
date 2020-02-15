#include <iostream>
#include <SFML/Graphics.hpp>
#include <sstream>
#include <SFML/Audio.hpp>

using namespace sf;

// Function declaration
void updateBranches(int seed);
int calculateFramesPerSecond(float fps);
// Global Var Declarations
const int NUM_FRAMES_PER_AVERAGE = 60;
float fpsHistory[NUM_FRAMES_PER_AVERAGE];
const int NUM_BRANCHES = 6;
Sprite branches[NUM_BRANCHES];
const int NUM_CLOUDS = 6;
Sprite clouds[NUM_CLOUDS];
bool cloudsActive[NUM_CLOUDS];
float cloudsSpeed[NUM_CLOUDS];
// Where is the player/branch?
// Left or Right
enum class side {LEFT,RIGHT,NONE};
side branchPositions[NUM_BRANCHES];

int main()
{
    // Create a video mode Object
    VideoMode vm(1920, 1080);

    // Create and open a window for the game
    RenderWindow window(vm, "Timber!!!", Style::None);

    // Create a texture to hold a graphic on the GPU
    Texture textureBackground;

    // Load a graphic into the texture
    textureBackground.loadFromFile("graphics/background.png");

    // Create a sprite
    Sprite spriteBackground;

    // Attach the texture to the sprite
    spriteBackground.setTexture(textureBackground);

    // Set the spriteBackground to cover the screen
    spriteBackground.setPosition(0, 0);

    // Make a tree sprite
    Texture textureTree;
    textureTree.loadFromFile("graphics/tree.png");
    Sprite spriteTree;
    spriteTree.setTexture(textureTree);
    spriteTree.setPosition(810, 0);

    // Prepare the bee
    Texture textureBee;
    textureBee.loadFromFile("graphics/bee.png");
    Sprite spriteBee;
    spriteBee.setTexture(textureBee);
    spriteBee.setPosition(0, 800);
    // Is the bee currently moving?
    bool beeActive = false;
    // How fast can the bee fly
    float beeSpeed = 0.0f;

    // Cloud Sprites
    Texture textureCloud;
    // Load texture
    textureCloud.loadFromFile("graphics/cloud.png");

    for (int i = 0; i < NUM_CLOUDS; i++) {
        clouds[i].setTexture(textureCloud);
        srand((int)time(0) * i);
        if ((rand() % 2) == 0) {
            clouds[i].setTextureRect(IntRect(300, 0, -300, 150));
        }
        clouds[i].setPosition(0, 0);
        cloudsActive[i] = false;
        cloudsSpeed[i] = 0.0f;
    }

    // Variables to control time itself
    Clock clock;
    Clock fpsClock;

    // Time bar
    RectangleShape timeBar;
    float timeBarStartWidth = 400;
    float timeBarHeight = 80;
    timeBar.setSize(Vector2f(timeBarStartWidth, timeBarHeight));
    timeBar.setFillColor(Color::Red);
    timeBar.setPosition((1920 / 2) - timeBarStartWidth / 2, 980);

    Time gameTimeTotal;
    float timeRemaining = 6.0f;
    float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;

    // Track whether the game is running
    bool paused = true;

    // FPS
    bool updateFPS = true;
    int frameTracker = 0;

    // Score
    int score = 0;
    int highScore = 0;

    // Draw some text
    Text highScoreText;
    Text messageText;
    Text scoreText;
    Text fpsText;

    // We need to choose a font
    Font font;
    font.loadFromFile("fonts/KOMIKAP_.ttf");

    // Set the font to our message
    highScoreText.setFont(font);
    messageText.setFont(font);
    scoreText.setFont(font);
    fpsText.setFont(font);

    // Assign the actual message
    messageText.setString("Press Enter to start!");
    scoreText.setString("Score = 0");
    fpsText.setString("0");
    highScoreText.setString("N/A");

    // Make it really big
    messageText.setCharacterSize(75);
    scoreText.setCharacterSize(100);
    fpsText.setCharacterSize(50);
    highScoreText.setCharacterSize(100);

    // Choose a color
    messageText.setFillColor(Color::White);
    scoreText.setFillColor(Color::White);
    fpsText.setFillColor(Color::Yellow);
    highScoreText.setFillColor(Color::Green);

    // Position the text
    FloatRect textRect = messageText.getLocalBounds();
    messageText.setOrigin(textRect.left +
        textRect.width / 2.0f,
        textRect.top +
        textRect.height / 2.0f);
    messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

    scoreText.setPosition(20, 20);

    highScoreText.setPosition(20, scoreText.getGlobalBounds().height + 40);

    fpsText.setPosition(1920 - fpsText.getGlobalBounds().width, 0);

    // Prepare 6 branches
    Texture textureBranch;
    textureBranch.loadFromFile("graphics/branch.png");
    // Set the texture for each branch sprite
    for (int i = 0; i < NUM_BRANCHES; i++) {
        branches[i].setTexture(textureBranch);
        branches[i].setPosition(-2000, -2000);
        // Set the sprite's origin to dead center
        // We can then spin it round without changing its position
        branches[i].setOrigin(220, 20);
    }

    // Prepare the player
    Texture texturePlayer;
    texturePlayer.loadFromFile("graphics/player.png");
    Sprite spritePlayer;
    spritePlayer.setTexture(texturePlayer);
    spritePlayer.setPosition(580, 720);

    // The player starts on the left
    side playerSide = side::LEFT;

    // Prepare the gravestone
    Texture textureRIP;
    textureRIP.loadFromFile("graphics/rip.png");
    Sprite spriteRIP;
    spriteRIP.setTexture(textureRIP);
    spriteRIP.setPosition(600, 860);

    // Prepare the axe
    Texture textureAxe;
    textureAxe.loadFromFile("graphics/axe.png");
    Sprite spriteAxe;
    spriteAxe.setTexture(textureAxe);
    spriteAxe.setPosition(700, 830);

    // Line the axe up with the tree
    const float AXE_POSITION_LEFT = 700;
    const float AXE_POSITION_RIGHT = 1075;

    // Prepare the flying log
    Texture textureLog;
    textureLog.loadFromFile("graphics/log.png");
    Sprite spriteLog;
    spriteLog.setTexture(textureLog);
    spriteLog.setPosition(810, 720);

    // Some other useful log related variables
    bool logActive = false;
    float logSpeedX = 1000;
    float logSpeedY = -1500;

    // Control player input
    bool acceptInput = false;

    // Prepare the sound
    SoundBuffer chopBuffer;
    chopBuffer.loadFromFile("sound/chop.wav");
    Sound chop;
    chop.setBuffer(chopBuffer);

    SoundBuffer deathBuffer;
    deathBuffer.loadFromFile("sound/death.wav");
    Sound death;
    death.setBuffer(deathBuffer);

    SoundBuffer ootBuffer;
    ootBuffer.loadFromFile("sound/out_of_time.wav");
    Sound outOfTime;
    outOfTime.setBuffer(ootBuffer);


    // Main Game Loop
    while (window.isOpen()) {
        /*
        ******************************
        Handle the players input
        ******************************
        */

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyReleased && !paused) {
                // Listen for key presses again
                acceptInput = true;

                //hide the axe
                spriteAxe.setPosition(2000, spriteAxe.getPosition().y);
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Escape)) {
            window.close();
        }
        // Start the game
        if (Keyboard::isKeyPressed(Keyboard::Return)) {
            paused = false;

            // Reset the time and the score
            score = 0;
            timeRemaining = 5;

            // Make all the branches disappear
            for (int i = 1; i < NUM_BRANCHES; i++) {
                branchPositions[i] = side::NONE;
            }

            // Make sure the gravestone is hidden
            spriteRIP.setPosition(675, 2000);

            // Move the player into position
            spritePlayer.setPosition(580, 720);
            acceptInput = true;
        }

        if (acceptInput) {
            // First handle pressing hte right cursor key
            if (Keyboard::isKeyPressed(Keyboard::Right)) {
                // Rotate Sprite to face tree
                if (playerSide == side::LEFT) {
                    spritePlayer.setTextureRect(IntRect(0, 0, 150, 192));
                }
                // Make sure the player is on the right
                playerSide = side::RIGHT;
                score++;

                // Add to the amount of time remaining
                timeRemaining += (2 / score) + .15;

                spriteAxe.setPosition(AXE_POSITION_RIGHT, spriteAxe.getPosition().y);

                spritePlayer.setPosition(1200, 720);
                
                // Update the branches
                updateBranches(score);

                // set the log flying to the left
                spriteLog.setPosition(810, 720);
                logSpeedX = -5000;
                logActive = true;

                acceptInput = false;

                chop.play();
            }
            // handle the left cursor key

            if (Keyboard::isKeyPressed(Keyboard::Left)) {
                // Rotate sprite to face tree
                if (playerSide == side::RIGHT) {
                    spritePlayer.setTextureRect(IntRect(150, 0, -150, 192));
                }
                // Make sure the player is on the right
                playerSide = side::LEFT;
                score++;

                // Add to the amount of time remaining
                timeRemaining += (2 / score) + .15;

                spriteAxe.setPosition(AXE_POSITION_LEFT, spriteAxe.getPosition().y);

                spritePlayer.setPosition(580, 720);

                // Update the branches
                updateBranches(score);

                // set the log flying to the left
                spriteLog.setPosition(810, 720);
                logSpeedX = 5000;
                logActive = true;

                acceptInput = false;
                chop.play();
            }
        }

        /*
        *****************************
        Update the scene
        *****************************
        */
        if (!paused) {
            // Measure time
            Time dt = clock.restart();

            // Subtract from the amount of time remaining
            timeRemaining -= dt.asSeconds();
            // size up the time bar
            timeBar.setSize(Vector2f(timeBarWidthPerSecond * timeRemaining, timeBarHeight));

            if (timeRemaining <= 0.0f) {
                // Pause the game
                paused = true;

                // Change the message shown to the player
                messageText.setString("Out of time!@!");

                // Reposition the text based on it's new size
                FloatRect textRect = messageText.getLocalBounds();
                messageText.setOrigin(
                    textRect.left + textRect.width / 2.0f,
                    textRect.top + textRect.height / 2.0f
                );
                messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);
                outOfTime.play();
            }

            // Setup the bee
            if (!beeActive) {
                // How fast is the bee
                srand((int)time(0) * 10);
                beeSpeed = (rand() % 200) + 200;

                // How high is the bee 
                srand((int)time(0) * 10);
                float height = (rand() % 500) + 500;
                spriteBee.setPosition(2000, height);
                beeActive = true;
            }
            else // Move the bee
            {
                spriteBee.setPosition(
                    spriteBee.getPosition().x -
                    (beeSpeed * dt.asSeconds()),
                    spriteBee.getPosition().y);
                // Has the bee reached the right hand edge of the screen?
                if (spriteBee.getPosition().x < -100) {
                    beeActive = false;
                }
            }

            // Manage the clouds
            for (int i = 0; i < NUM_CLOUDS; i++) {
                int randInc = i + 1;
                if (!cloudsActive[i]) {
                    srand((int)time(0)* (randInc * 10));
                    cloudsSpeed[i] = (rand() % (randInc * 100) + (randInc ^ 2));
                    clouds[i].setPosition(-200, (rand() % 150) + (25 * randInc));
                    cloudsActive[i] = true;
                }
                else {
                    clouds[i].setPosition(
                        clouds[i].getPosition().x +
                        (cloudsSpeed[i] * dt.asSeconds()),
                        clouds[i].getPosition().y
                    );
                    if (clouds[i].getPosition().x > 1920) {
                        cloudsActive[i] = false;
                    }
                }
            }

            // Update the score text
            std::stringstream ss;
            ss << "Score = " << score;
            scoreText.setString(ss.str());

            // update the branch sprites
            for (int i = 0; i < NUM_BRANCHES; i++) {
                float height = i * 150;
                if (branchPositions[i] == side::LEFT) {
                    // Move the sprite to the left side
                    branches[i].setPosition(610, height);
                    // Flip the sprite around the other way 
                    branches[i].setRotation(180);
                }
                else if (branchPositions[i] == side::RIGHT) {
                    // Move the sprite to the right side
                    branches[i].setPosition(1330, height);
                    // Set the sprite rotation to normal
                    branches[i].setRotation(0);
                }
                else
                {
                    // Hide the branch 
                    branches[i].setPosition(3000, height);
                }
            }

            // Handle a flying log
            if (logActive) {
                spriteLog.setPosition(
                    spriteLog.getPosition().x + (logSpeedX * dt.asSeconds()),
                    spriteLog.getPosition().y + (logSpeedY * dt.asSeconds())
                );
                // Has the log reached the right hand edge?
                if (spriteLog.getPosition().x < -100 || spriteLog.getPosition().x > 2000) {
                    logActive = false;
                    spriteLog.setPosition(810, 720);
                }
            }

            // Has the player been squished by a branch?
            if (branchPositions[5] == playerSide) {
                // death
                paused = true;
                acceptInput = false;
                // Draw gravestone
                spriteRIP.setPosition(525, 760);

                //hide player
                spritePlayer.setPosition(2000, 660);

                //Change text message  
                messageText.setString("Squished!");

                FloatRect textRect = messageText.getLocalBounds();
                messageText.setOrigin(textRect.left + textRect.width / 2.0f,
                    textRect.top + textRect.height / 2.0f);
                messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

                death.play();

                if (score > highScore) {
                    highScore = score;
                }
                std::stringstream hs;
                hs << highScore;
                highScoreText.setString(hs.str());
            }

        } // End if(!paused)
        

        /*
        *****************************
        Draw the scene
        *****************************
        */
        // Clear everything from the last frame
        window.clear();
    
        // Update FPS
        if (frameTracker >= NUM_FRAMES_PER_AVERAGE) {
            frameTracker = 0;
            updateFPS = true;
        }
        else 
        {
            frameTracker++;
            updateFPS = false;
        }

        Time fpsTime = fpsClock.restart();
        float fps = 1.0f / fpsTime.asSeconds();
        if (updateFPS) {
            std::stringstream fpsss;
            fpsss << calculateFramesPerSecond(fps);
            fpsText.setString(fpsss.str());
            // adjust fpsText position
            fpsText.setPosition(1920 - fpsText.getGlobalBounds().width, 0);
        }
        else
        {
            calculateFramesPerSecond(fps);
        }

        // Draw our game scene here
        window.draw(spriteBackground);

        // Draw the clouds
        for (int i = 0; i < NUM_CLOUDS; i++) {
            window.draw(clouds[i]);
        }
        /*window.draw(spriteCloud1);
        window.draw(spriteCloud2);
        window.draw(spriteCloud3);*/

        //  Draw the branches
        for (int i = 0; i < NUM_BRANCHES; i++) {
            window.draw(branches[i]);
        }

        // Draw the tree   
        window.draw(spriteTree);
        
        // Draw the player
        window.draw(spritePlayer);

        // Draw the axe
        window.draw(spriteAxe);

        // Draw the flying log
        window.draw(spriteLog);

        // Draw the gravestone
        window.draw(spriteRIP);

        // Draw the bee
        window.draw(spriteBee);

        // Draw the score
        window.draw(scoreText);

        // Draw high score
        window.draw(highScoreText);

        // Draw the timebar
        window.draw(timeBar);

        if (paused) {
            // Draw our message
            window.draw(messageText);
        }

        // Draw FPS
         window.draw(fpsText);
       
        

        // Show everything we just drew
        window.display();
    }
    return 0;
}

int calculateFramesPerSecond(float fps) {
    float fpsAverage = 0.0f;
    for (int j = NUM_FRAMES_PER_AVERAGE - 1; j > 0; j--) {
        fpsHistory[j] = fpsHistory[j - 1];
        fpsAverage += fpsHistory[j];
    }
    fpsHistory[0] = fps;
    fpsAverage += fps;
    return fpsAverage / NUM_FRAMES_PER_AVERAGE;
}

// Function definition
void updateBranches(int seed) {
    // Move all the branches down one place
    for (int j = NUM_BRANCHES - 1; j > 0; j--) {
        branchPositions[j] = branchPositions[j - 1];
    }
    // Spawn a new branch at position 0
    // LEFT, RIGHT, NONE
    srand((int)time(0) + seed);
    int r = (rand() % 5);
    switch (r) {
    case 0:
        branchPositions[0] = side::LEFT;
        break;
    case 1:
        branchPositions[0] = side::RIGHT;
        break;
    default:
        branchPositions[0] = side::NONE;
        break;
    }
}