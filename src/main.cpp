#include "raylib.h" // Base Raylib header
#include "raymath.h" // Vector math
#include <stdint.h>
#include <stdio.h> // printf
#include <assert.h> // assert
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <vector>
#include <algorithm>

#define MAX_SOUNDS 10

//start draw shapes on x,y
int startX = 104;
int startY = 24;

// config
int dificult = 4;
int ground = 2;

// numbers gui
int score = 0;
int level = 1;
int exploded = 0;

static const int screenWidth = 256;
static const int screenHeight = 192;

static const int SPRITE_SIZE_X = 8;
static const int SPRITE_SIZE_Y = 8;

static const int TABLE_SIZE_X = 6;
static const int TABLE_SIZE_Y = 18;

int table[TABLE_SIZE_Y][TABLE_SIZE_X] = {};

bool hasMovement = true;
bool hasDestruction = false;
bool hasPlayer = false;
bool gameover = false;
bool selectScreen = true;

int playerStartX = 2;
int playerStartY = 2;

typedef struct Person {
    float x;
    float y;
    int sprite1;
    int sprite2;
    int sprite3;
} Person;

Person player;
Person next;

float flash = 0;
int flashCounter = 0;
int explosionCounter = 0;

typedef struct Point {
    int x, y;
} Point;
std::vector<Point> boom;

typedef struct Explosion {
    float x;
    float y;
    int frame;
    bool active;
} Explosion;

std::vector<Explosion> explosions;

int lineExplode = 0;
float intervalToDownPlayer = 0.5f;
float intervalToRefreshTable = 0.1f;

Sound popBlock;
Sound collisonBlock;
Sound changeBlock;
Sound upLevel;
Sound musicGameOver;

Music music;
Music musicIntro;

float pitch = 1.0f;
bool HasMusicGameHoverPlayed = false;

Texture background;
Texture blocks;
Texture explode;

char txt[100] = "";

void drawSpriteSheetTile(
    const Texture texture,
    const int spriteX,
    const int spriteY,
    const int spriteSize,
    const Vector2 position,
    const Vector2 scale = { 1, 1 }
) {
    DrawTextureRec(
        texture,
        { (float)(spriteX * spriteSize), (float)(spriteY * spriteSize), (float)spriteSize * scale.x, (float)spriteSize * scale.y},
        position,
        WHITE);
}

void drawTable()
{
    int sprite = 0;

    for(int y=0; y < TABLE_SIZE_Y; y++)
    {
        for(int x=0; x < TABLE_SIZE_X; x++)
        {
            sprite = table[y][x];

            if(sprite>0) drawSpriteSheetTile(blocks, sprite-1, 0, SPRITE_SIZE_X, { (float)(startX + (x*SPRITE_SIZE_X)), (float)(startY + (y*SPRITE_SIZE_Y)) });
        }
    }
}

void moveTable()
{
    hasMovement = false;
    for(int y=TABLE_SIZE_Y-1; y >= 0; y--)
    {
        for(int x=TABLE_SIZE_X-1; x >= 0; x--)
        {
            if(table[y-1][x]>0 && table[y][x]==0){
                table[y][x] = table[y-1][x];
                table[y-1][x] = 0;
                hasMovement = true;
            }
        }
    }
}

void destroyShapes(float delta)
{
    if(hasMovement) return;



    if(!hasDestruction){
        boom.clear();
        for(int y=TABLE_SIZE_Y-1; y >= 0; y--)
        {
            for(int x=TABLE_SIZE_X-1; x >= 0; x--)
            {
                int sprite = table[y][x];
                if(sprite>0){

                    // vertical
                    if(y>=2 && table[y][x]==sprite && table[y-1][x]==sprite && table[y-2][x]==sprite){
                        boom.push_back({x,y});
                        boom.push_back({x,y-1});
                        boom.push_back({x,y-2});
                        hasDestruction = true;
                    }

                    // horizontal
                    if(x>=2 && table[y][x]==sprite && table[y][x-1]==sprite && table[y][x-2]==sprite){
                        boom.push_back({x,y});
                        boom.push_back({x-1,y});
                        boom.push_back({x-2,y});
                        hasDestruction = true;
                    }

                    // diagonal left
                    if(x>=2 && y>=2 && table[y][x]==sprite && table[y-1][x-1]==sprite && table[y-2][x-2]==sprite){
                        boom.push_back({x,y});
                        boom.push_back({x-1,y-1});
                        boom.push_back({x-2,y-2});
                        hasDestruction = true;
                    }

                    // diagonal right
                    if(y>=2 && x<=3 && table[y][x]==sprite && table[y-1][x+1]==sprite && table[y-2][x+2]==sprite){
                        boom.push_back({x,y});
                        boom.push_back({x+1,y-1});
                        boom.push_back({x+2,y-2});
                        hasDestruction = true;
                    }
                }
            }
        }
    }

    if(hasDestruction){
        flash += delta*5.0f;
        if(flash>=1 && flash<=2){
            for (size_t i = 0; i < boom.size(); ++i) {
                    DrawRectangleRec({(float)(startX + (boom[i].x*SPRITE_SIZE_X)), (float)(startY + (boom[i].y*SPRITE_SIZE_Y)), SPRITE_SIZE_X, SPRITE_SIZE_Y}, BLACK);
            }

        }
        if(flash>=2 && flashCounter<3) { flash = 0; flashCounter +=1; }

        if(flashCounter==3){

            for (size_t i = 0; i < boom.size(); ++i) {
                explosions.push_back( Explosion{(float)boom[i].x, (float)boom[i].y, 0, true} );
                explosionCounter += 1;
            }

            flashCounter +=1;
        }

        if(flashCounter==4){
            for(auto& explosion : explosions){
                DrawRectangleRec({(float)(startX + (explosion.x*SPRITE_SIZE_X)), (float)(startY + (explosion.y*SPRITE_SIZE_Y)), SPRITE_SIZE_X, SPRITE_SIZE_Y}, BLACK);
                drawSpriteSheetTile(explode, explosion.frame, 0, SPRITE_SIZE_X, {(float)(startX + (explosion.x*SPRITE_SIZE_X)), (float)(startY + (explosion.y*SPRITE_SIZE_Y))});
                explosion.frame += 1;
                if(explosion.frame>3) {
                    //explosion.frame = 0;
                    explosion.active=false;
                    table[(int)explosion.y][(int)explosion.x] = 0;

                    exploded += 1;
                }
            }



            if(explosions.size()==0){

                boom.clear();
                hasDestruction = false;

                flashCounter = 0;
                flash = 0;
                PlaySound(popBlock);

                if(explosionCounter==3) score += 30;
                if(explosionCounter==4) score += 40;
                if(explosionCounter==5) score += 70;
                if(explosionCounter==6) score += 80;
                if(explosionCounter>=7) score += 120;

                explosionCounter = 0;
            }

            explosions.erase(std::remove_if(explosions.begin(), explosions.end(),[](Explosion& e){ return e.active==false; }), explosions.end());
        }
    }
}

int DestructAll(int limit)
{
    if(limit<=0){
        limit = 0;
        return limit;
    }

    for(int y=limit; y >= limit-1; y--)
    {
        for(int x=TABLE_SIZE_X-1; x >= 0; x--)
        {
            explosions.push_back( Explosion{(float)x, (float)y, 0, true} );
        }
    }

    for(auto& explosion : explosions){
        DrawRectangleRec({(float)(startX + (explosion.x*SPRITE_SIZE_X)), (float)(startY + (explosion.y*SPRITE_SIZE_Y)), SPRITE_SIZE_X, SPRITE_SIZE_Y}, BLACK);
        drawSpriteSheetTile(explode, explosion.frame, 0, SPRITE_SIZE_X, {(float)(startX + (explosion.x*SPRITE_SIZE_X)), (float)(startY + (explosion.y*SPRITE_SIZE_Y))});

        explosion.frame += 1;

        if(explosion.frame>3) explosion.active=false;
    }

    explosions.erase(std::remove_if(explosions.begin(), explosions.end(),[](Explosion& e){ return e.active==false; }), explosions.end());

    limit -= 1;

    return limit;
}

void newPlayer(bool create)
{
    if(hasMovement) return;
    if(hasDestruction) return;
    if(hasPlayer) return;

    if(!create) return;

    if(table[playerStartY][playerStartX]!=0){
        gameover = true;
        return;
    }

    player.sprite1 = next.sprite1;
    player.sprite2 = next.sprite2;
    player.sprite3 = next.sprite3;

    next.sprite1 = (rand() % dificult)+1;
    next.sprite2 = (rand() % dificult)+1;
    next.sprite3 = (rand() % dificult)+1;

    player.x = playerStartX;
    player.y = playerStartY;

    hasPlayer = true;
}

void drawPlayer()
{
    if(hasPlayer){
        drawSpriteSheetTile(blocks, player.sprite1-1, 0, SPRITE_SIZE_X, {(float)(startX + (player.x*SPRITE_SIZE_X)), (float)(startY + ((player.y-2)*SPRITE_SIZE_Y))});

        drawSpriteSheetTile(blocks, player.sprite2-1, 0, SPRITE_SIZE_X, {(float)(startX + (player.x*SPRITE_SIZE_X)), (float)(startY + ((player.y-1)*SPRITE_SIZE_Y))});

        drawSpriteSheetTile(blocks, player.sprite3-1, 0, SPRITE_SIZE_X, {(float)(startX + (player.x*SPRITE_SIZE_X)), (float)(startY + (player.y*SPRITE_SIZE_Y))});
    }
}

void drawNext()
{
    drawSpriteSheetTile(blocks, next.sprite1-1, 0, SPRITE_SIZE_X, {70, 25});
    drawSpriteSheetTile(blocks, next.sprite2-1, 0, SPRITE_SIZE_X, {70, 33});
    drawSpriteSheetTile(blocks, next.sprite3-1, 0, SPRITE_SIZE_X, {70, 41});
}

void movePlayer(bool move)
{
    if(!hasPlayer) return;

    if(table[(int)player.y+1][(int)player.x]==0 && player.y<TABLE_SIZE_Y-1){
        if(move || IsKeyDown(KEY_DOWN)) {
            player.y += 1;
            if(player.y>TABLE_SIZE_Y-1) player.y = TABLE_SIZE_Y-1;
        }
    } else {
        table[(int)player.y-2][(int)player.x] = player.sprite1;
        table[(int)player.y-1][(int)player.x] = player.sprite2;
        table[(int)player.y][(int)player.x] = player.sprite3;

        hasPlayer = false;
        PlaySound(collisonBlock);
    }

    if (hasPlayer && IsKeyPressed(KEY_RIGHT) && table[(int)player.y][(int)player.x+1]==0) player.x += 1;
    if (hasPlayer && IsKeyPressed(KEY_LEFT) && table[(int)player.y][(int)player.x-1]==0) player.x -= 1;

    if(player.x < 0) player.x = 0;
    if(player.x > 5) player.x = 5;

    if(hasPlayer && (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP))){
        PlaySound(changeBlock);

        int sp1 = player.sprite1;
        int sp2 = player.sprite2;
        int sp3 = player.sprite3;

        player.sprite1 = sp2;
        player.sprite2 = sp3;
        player.sprite3 = sp1;
    }
}

void resetGame()
{
    for(int y=0; y < TABLE_SIZE_Y; y++)
    {
        for(int x=0; x < TABLE_SIZE_X; x++)
        {
            int sprite = (rand() % dificult)+1;
                if(y<(TABLE_SIZE_Y-ground)) sprite = 0;
            table[y][x] = sprite;
        }
    }

    // first player blocks
    next.sprite1 = (rand() % dificult)+1;
    next.sprite2 = (rand() % dificult)+1;
    next.sprite3 = (rand() % dificult)+1;

    lineExplode = TABLE_SIZE_Y - 1;

    hasDestruction = false;
    hasMovement = false;
    hasPlayer = false;
    gameover = false;

    level = 1;
    score = 0;
    exploded = 0;

    pitch = 1.0f;

    HasMusicGameHoverPlayed = false;

    StopMusicStream(musicIntro);

    PlayMusicStream(music);
    SetMusicVolume(music, 0.6);

    explosions.clear();

    intervalToDownPlayer = 0.5f;
    intervalToRefreshTable = 0.1f;

    selectScreen = false;
}

void changeConfig()
{
    DrawText("Dificult", 175, 40, 1, WHITE);
    DrawText("Ground", 175, 80, 1, WHITE);

    sprintf(txt, "%i", ground);
    DrawText(txt, 185, 95, 1, WHITE);

    for(int i=0; i<dificult; i++){
        drawSpriteSheetTile(blocks, i, 0, SPRITE_SIZE_X, {(float)(185+(i*SPRITE_SIZE_X)), 55});
    }

    if(IsKeyPressed(KEY_RIGHT)) dificult += 1;
    if(IsKeyPressed(KEY_LEFT)) dificult -= 1;
    if(dificult<4) dificult = 4;
    if(dificult>6) dificult = 6;

    if(IsKeyPressed(KEY_UP)) ground += 1;
    if(IsKeyPressed(KEY_DOWN)) ground -= 1;
    if(ground<0) ground = 0;
    if(ground>12) ground = 12;
}

int main(void)
{
    srand(time(0));

    float timeToRefreshTable = 0.0f;
    float timeToDownPlayer = 0.0f;

    InitWindow(screenWidth*3, screenHeight*3, "Columns");
    SetTargetFPS(60); // habilita 60 FPS se possivel

    InitAudioDevice();

    background = LoadTexture("../resources/gfx/background.png");
    blocks = LoadTexture("../resources/gfx/sprites/blocks.png");
    explode = LoadTexture("../resources/gfx/sprites/explode.png");

    Texture gameoverLabel = LoadTexture("../resources/gfx/game-over.png");


    popBlock = LoadSound("../resources/sfx/09.mp3");
    collisonBlock = LoadSound("../resources/sfx/11.mp3");
    changeBlock = LoadSound("../resources/sfx/14.mp3");
    upLevel = LoadSound("../resources/sfx/12.mp3");

    music = LoadMusicStream("../resources/music/01.mp3");
    musicIntro = LoadMusicStream("../resources/music/intro.mp3");
    musicGameOver = LoadSound("../resources/music/game-over.mp3");

    RenderTexture pixelartRenderTexture = LoadRenderTexture(screenWidth, screenHeight);

    // Minimum window size
    if (GetScreenWidth() < screenWidth) {
        SetWindowSize(screenWidth, GetScreenHeight());
    }
    if (GetScreenHeight() < screenHeight) {
        SetWindowSize(GetScreenWidth(), screenHeight);
    }


    //resetGame();

    while(!WindowShouldClose())
    {
        const float delta = Clamp(GetFrameTime(), 0.0001f, 0.1f);
        timeToRefreshTable += delta;
        timeToDownPlayer += delta;

        UpdateMusicStream(music);
        UpdateMusicStream(musicIntro);

        {
            BeginTextureMode(pixelartRenderTexture);
            ClearBackground(BLACK);

            DrawTextureRec( background, { 0, 0, 256, 192 }, { 0, 0 }, WHITE);

            if(selectScreen){
                StopMusicStream(music);

                if(IsMusicStreamPlaying(musicIntro)==false){
                    PlayMusicStream(musicIntro);
                    SetMusicVolume(musicIntro, 0.6);
                }

                changeConfig();

                DrawText("Press ENTER", 175, 140, 1, YELLOW);
                if(IsKeyPressed(KEY_ENTER)) resetGame();
            } else {
                if(gameover==false){
                    drawTable();

                    drawNext();

                    if(timeToRefreshTable >= intervalToRefreshTable){
                        moveTable();
                        timeToRefreshTable = 0.0f;
                    }

                    destroyShapes(delta);

                    newPlayer(timeToDownPlayer >= intervalToDownPlayer);
                    drawPlayer();

                    movePlayer(timeToDownPlayer >= intervalToDownPlayer);

                    if(timeToDownPlayer >= intervalToDownPlayer) timeToDownPlayer = 0.0f;
                } else {
                    StopMusicStream(music);

                    if(HasMusicGameHoverPlayed==false){
                        PlaySound(musicGameOver);
                        HasMusicGameHoverPlayed = true;
                    }

                    lineExplode = DestructAll(lineExplode);

                    DrawTextureRec( gameoverLabel, { 0, 0, 46, 23 }, { 105, 70 }, WHITE);

                    if(timeToRefreshTable >= 5.0f){
                        selectScreen = true;
                        timeToRefreshTable = 0.0f;
                    }
                }
            }

            sprintf(txt, "%i", score);
            DrawText(txt, 60, 100, 2, WHITE);

            sprintf(txt, "%i", exploded);
            DrawText(txt, 60, 140, 2, WHITE);

            sprintf(txt, "%i", level);
            DrawText(txt, 60, 180, 2, WHITE);

            SetMusicPitch(music, pitch);

            if(score/(level*1000) > level){
                PlaySound(upLevel);
                pitch += 0.025f;
                level += 1;
                intervalToRefreshTable -= 0.E1f;
            }

            if(IsKeyPressed(KEY_ONE)) blocks = LoadTexture("../resources/gfx/sprites/blocks.png");
            if(IsKeyPressed(KEY_TWO)) blocks = LoadTexture("../resources/gfx/sprites/dices.png");
            if(IsKeyPressed(KEY_THREE)) blocks = LoadTexture("../resources/gfx/sprites/fruits.png");
            if(IsKeyPressed(KEY_FOUR)) blocks = LoadTexture("../resources/gfx/sprites/gems.png");
            if(IsKeyPressed(KEY_FIVE)) blocks = LoadTexture("../resources/gfx/sprites/shapes.png");


            EndTextureMode();
        }

        {
            BeginDrawing();
            ClearBackground(BLACK);

            const Vector2 window = { (float)GetScreenWidth(), (float)GetScreenHeight() };
            const float scale = fmaxf(1.0f, floorf(fminf(window.x / screenWidth, window.y / screenHeight)));
            const Vector2 size = { scale * screenWidth, scale * screenHeight };
            const Vector2 offset = Vector2Scale(Vector2Subtract(window, size), 0.5);

            DrawTexturePro(
                pixelartRenderTexture.texture,
                { 0, 0, (float)pixelartRenderTexture.texture.width, -(float)pixelartRenderTexture.texture.height },
                { offset.x, offset.y, size.x, size.y },
                {}, 0, WHITE);




            EndDrawing();
        }
    }

    UnloadTexture(background);
    UnloadTexture(blocks);
    UnloadTexture(explode);
    UnloadTexture(gameoverLabel);

    UnloadSound(popBlock);
    UnloadSound(collisonBlock);
    UnloadSound(changeBlock);
    UnloadSound(musicGameOver);

    UnloadMusicStream(music);
    UnloadMusicStream(musicIntro);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
