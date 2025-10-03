#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <iostream>
#include <list>
#include <string>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <btBulletDynamicsCommon.h>

// Game states
enum GameState {
    STATE_GAMEPLAY,
    STATE_SETTINGS,
    STATE_EDITOR
};

// Struct for static level geometry
struct PhysicsBlock {
    btRigidBody* body;
    Color color;
    Vector3 size;
};

// Struct for visual effects like bullet tracers
struct VisualEffect {
    Vector3 start;
    Vector3 end;
    float life;
    Color color;
};

// Player/Enemy state for handling different actions
enum CharacterState {
    STATE_ON_GROUND,
    STATE_JUMPING,
    STATE_DYING,
};

// Player data
struct Player {
    btRigidBody* body;
    int health;
    int maxHealth;
    CharacterState state;
    float modelRotationAngle;
    bool isMoving;
};

// Struct for enemies/NPCs
struct Enemy {
    btRigidBody* body;
    int health;
    int maxHealth;
    CharacterState state;
    float animationTimer;
    bool isMoving;
    float modelRotationAngle;
    float deathTimer;

    // AI state
    float aiStateTimer;
    Vector3 wanderDirection;
};

// Colors for drawing characters
struct CharacterColors {
    Color skin;
    Color shirt;
    Color pants;
};

// Editor modes
enum EditorMode {
    MODE_PLACE,
    MODE_ERASE,
    MODE_SPAWN
};


// Settings controllable from the menu
struct GameSettings {
    bool enemiesFrozen = false;
    bool enemiesAttack = false;
    bool enemiesSuperSpeed = false;
    bool playerInfiniteHealth = false;
    bool playerInfiniteAmmo = false;
    bool inEditMode = false;

    // Editor settings
    EditorMode editorMode = MODE_PLACE;
    Vector3 editorBlockSize = { 2, 2, 2 };
    Color editorBlockColor = ORANGE;
};


// Function to create a standalone static physics box.
void CreateStaticBox(btDiscreteDynamicsWorld* dynamicsWorld, std::list<PhysicsBlock>& worldBlocks, Vector3 position, Vector3 size, Color color) {
    btCollisionShape* shape = new btBoxShape(btVector3(size.x * 0.5f, size.y * 0.5f, size.z * 0.5f));
    btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y, position.z)));
    btRigidBody::btRigidBodyConstructionInfo ci(0, motionState, shape, btVector3(0, 0, 0));
    btRigidBody* body = new btRigidBody(ci);
    body->setFriction(1.0f);

    worldBlocks.push_back({ body, color, size });
    body->setUserPointer(&worldBlocks.back());
    body->setUserIndex(-1); // Use -1 for static world objects

    dynamicsWorld->addRigidBody(body);
}

std::vector<Vector3> enemySpawnPoints;

// Function to create an enemy
void CreateEnemy(btDiscreteDynamicsWorld* dynamicsWorld, std::list<Enemy>& enemies, Vector3 position) {
    float enemyHeight = 2.0f;
    btCollisionShape* enemyShape = new btCapsuleShape(0.4f, enemyHeight - 0.8f);
    btDefaultMotionState* enemyMotion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(position.x, position.y, position.z)));
    btScalar enemyMass = 1.0f;
    btVector3 enemyInertia(0, 0, 0);
    enemyShape->calculateLocalInertia(enemyMass, enemyInertia);
    btRigidBody::btRigidBodyConstructionInfo enemyCI(enemyMass, enemyMotion, enemyShape, enemyInertia);
    btRigidBody* body = new btRigidBody(enemyCI);
    body->setAngularFactor(0);
    body->setActivationState(DISABLE_DEACTIVATION);
    body->setFriction(1.0f);

    enemies.push_front({}); // Add new enemy to the front of the list
    Enemy& newEnemy = enemies.front();
    newEnemy.body = body;
    newEnemy.maxHealth = 100;
    newEnemy.health = 100;
    newEnemy.state = STATE_ON_GROUND;
    newEnemy.animationTimer = (float)GetRandomValue(0, 100) / 10.0f;
    newEnemy.isMoving = false;
    newEnemy.modelRotationAngle = 0;
    newEnemy.deathTimer = 0.0f;
    newEnemy.aiStateTimer = (float)GetRandomValue(1, 3);
    newEnemy.wanderDirection = { 0, 0, 0 };

    body->setUserPointer(&newEnemy);
    body->setUserIndex(2); // Use 2 for enemies

    dynamicsWorld->addRigidBody(body);
}

// Creates a large shooter arena with cover objects and enemies.
void CreateShooterArena(btDiscreteDynamicsWorld* dynamicsWorld, std::list<PhysicsBlock>& worldBlocks, std::list<Enemy>& enemies) {
    // --- Main Floor ---
    CreateStaticBox(dynamicsWorld, worldBlocks, { 0, -0.5f, 0 }, { 80, 1, 80 }, GRAY);

    // --- Boundary Walls ---
    CreateStaticBox(dynamicsWorld, worldBlocks, { 0, 2.0f, 40.5f }, { 80, 5, 1 }, DARKGRAY);
    CreateStaticBox(dynamicsWorld, worldBlocks, { 0, 2.0f, -40.5f }, { 80, 5, 1 }, DARKGRAY);
    CreateStaticBox(dynamicsWorld, worldBlocks, { 40.5f, 2.0f, 0 }, { 1, 5, 80 }, DARKGRAY);
    CreateStaticBox(dynamicsWorld, worldBlocks, { -40.5f, 2.0f, 0 }, { 1, 5, 80 }, DARKGRAY);

    // --- Arena Objects ---
    CreateStaticBox(dynamicsWorld, worldBlocks, { 0, 1.0f, 15 }, { 20, 2, 2 }, DARKGRAY);
    CreateStaticBox(dynamicsWorld, worldBlocks, { -15, 1.0f, 5 }, { 3, 2, 3 }, DARKGRAY);
    CreateStaticBox(dynamicsWorld, worldBlocks, { -12, 1.0f, -10 }, { 3, 2, 3 }, DARKGRAY);
    CreateStaticBox(dynamicsWorld, worldBlocks, { 20, 1.0f, -5 }, { 10, 2, 2 }, DARKGRAY);
    CreateStaticBox(dynamicsWorld, worldBlocks, { 24, 1.0f, -10 }, { 2, 2, 10 }, DARKGRAY);
    CreateStaticBox(dynamicsWorld, worldBlocks, { 0, 2.0f, -20 }, { 4, 4, 4 }, DARKGRAY);

    // --- Define Spawn Points & Spawn Enemies ---
    enemySpawnPoints.push_back({ 0, 2.0f, 25 });
    enemySpawnPoints.push_back({ 20, 2.0f, -25 });
    enemySpawnPoints.push_back({ -25, 2.0f, 0 });
    enemySpawnPoints.push_back({ 15, 2.0f, 10 });
    enemySpawnPoints.push_back({ -15, 2.0f, -20 });

    for (const auto& sp : enemySpawnPoints) {
        CreateEnemy(dynamicsWorld, enemies, sp);
    }
}


// Draws a procedurally animated character with a skeletal hierarchy and weapon handling.
void DrawAnimatedCharacter(Vector3 position, float rotationAngle, CharacterState state, float animationTimer, bool isMoving, bool isAiming, bool isTalking, float reloadProgress, float fireRecoil, CharacterColors colors) {
    Color weaponColor = { 40, 40, 40, 255 };
    Color weaponDetail = { 60, 60, 60, 255 };

    float walkCycleSpeed = 10.0f;
    float walkCycleAmplitude = 35.0f;
    float idleBreathSpeed = 2.0f;
    float idleBreathAmplitude = 0.02f;

    float leftLegRot = 0, rightLegRot = 0, leftArmRotX = 0, rightArmRotX = 0;
    float leftArmRotZ = 0, rightArmRotZ = 0;
    float torsoYOffset = 0, torsoXRot = 0, bodyRotX = 0, headRotX = 0;

    // Base pose based on movement state
    switch (state) {
    case STATE_ON_GROUND:
        if (isMoving) {
            leftLegRot = sinf(animationTimer * walkCycleSpeed) * walkCycleAmplitude;
            rightLegRot = -sinf(animationTimer * walkCycleSpeed) * walkCycleAmplitude;
        }
        else {
            if (isTalking) {
                // Faster, more pronounced idle animation for talking
                torsoYOffset = sinf(animationTimer * idleBreathSpeed * 2.5f) * idleBreathAmplitude * 1.8f;
                headRotX = cosf(animationTimer * idleBreathSpeed * 1.5f) * 5.0f; // Nodding motion
            }
            else {
                // Regular idle animation
                torsoYOffset = sinf(animationTimer * idleBreathSpeed) * idleBreathAmplitude;
            }
        }
        break;
    case STATE_JUMPING:
        leftLegRot = 45.0f; rightLegRot = -20.0f;
        break;
    case STATE_DYING:
        bodyRotX = 90.0f; // Lay flat on the ground
        torsoYOffset = -1.0f;
        break;
    }

    // --- Arm Animation Overrides for Weapon Handling ---
    if (state != STATE_DYING) {
        if (isTalking) {
            // Talking gesture
            rightArmRotX = 25.0f + sinf(animationTimer * 4.0f) * 10.0f;
            rightArmRotZ = -45.0f;
            leftArmRotX = 35.0f + cosf(animationTimer * 3.0f) * 10.0f;
            leftArmRotZ = 45.0f;
        }
        else if (isAiming) {
            torsoXRot = -10.0f; // Lean into the sight
            rightArmRotX = 90.0f; rightArmRotZ = -20.0f;
            leftArmRotX = 90.0f; leftArmRotZ = 20.0f;
        }
        else { // Default weapon holding pose
            rightArmRotX = 75.0f; rightArmRotZ = -15.0f;
            leftArmRotX = 80.0f; leftArmRotZ = 20.0f;
        }
    }
    else { // Ragdoll arms on death
        rightArmRotX = -45.0f; rightArmRotZ = 70.0f;
        leftArmRotX = 45.0f; leftArmRotZ = -70.0f;
    }

    // Reloading animation override
    if (reloadProgress > 0) {
        float progress = 1.0f - (reloadProgress / 2.0f);
        float reloadAngle = sinf(progress * PI * 2) * -15.0f;
        leftArmRotX += reloadAngle;
        leftArmRotZ += reloadAngle;
    }

    // Apply recoil
    rightArmRotX -= fireRecoil * 15.0f;
    leftArmRotX -= fireRecoil * 10.0f;
    rightArmRotZ += fireRecoil * 5.0f;

    rlPushMatrix();
    rlTranslatef(position.x, position.y, position.z);
    rlRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);
    rlRotatef(bodyRotX, 1.0f, 0.0f, 0.0f);

    // --- Torso & Head ---
    rlPushMatrix();
    rlTranslatef(0, torsoYOffset, 0);
    rlRotatef(torsoXRot, 1.0, 0, 0);
    DrawCubeV({ 0.0f, 0.4f, 0.0f }, { 0.7f, 0.8f, 0.4f }, colors.shirt);
    // Head with talking animation
    rlPushMatrix();
    rlRotatef(headRotX, 1.0, 0, 0);
    DrawCubeV({ 0.0f, 1.05f, 0.0f }, { 0.5f, 0.5f, 0.5f }, colors.skin);
    rlPopMatrix();
    rlPopMatrix();

    // --- Legs ---
    rlPushMatrix();
    rlTranslatef(-0.18f, -0.1f + torsoYOffset, 0.0f);
    rlRotatef(leftLegRot, 1.0f, 0.0f, 0.0f);
    DrawCubeV({ 0, -0.5f, 0 }, { 0.3f, 1.0f, 0.3f }, colors.pants);
    rlPopMatrix();

    rlPushMatrix();
    rlTranslatef(0.18f, -0.1f + torsoYOffset, 0.0f);
    rlRotatef(rightLegRot, 1.0f, 0.0f, 0.0f);
    DrawCubeV({ 0, -0.5f, 0 }, { 0.3f, 1.0f, 0.3f }, colors.pants);
    rlPopMatrix();

    // --- Arms and Weapon ---
    rlPushMatrix();
    rlTranslatef(0.3f, 0.75f + torsoYOffset, 0.05f);
    rlRotatef(torsoXRot, 1.0, 0, 0);
    rlRotatef(rightArmRotZ, 0.0f, 0.0f, 1.0f);
    rlRotatef(rightArmRotX, 1.0f, 0.0f, 0.0f);
    DrawCubeV({ 0, -0.35f, 0 }, { 0.2f, 0.7f, 0.2f }, colors.shirt);

    // Weapon Model
    if (state != STATE_DYING && !isTalking) {
        rlPushMatrix();
        rlTranslatef(0.0f, -0.4f, 0.25f);
        rlRotatef(90, 0, 1, 0);
        if (isAiming) {
            rlTranslatef(0.2f, 0.1f, -0.15f);
        }
        DrawCubeV({ 0.0f, 0.0f, 0.0f }, { 0.8f, 0.16f, 0.13f }, weaponColor);
        DrawCubeV({ -0.15f, -0.12f, 0.0f }, { 0.18f, 0.25f, 0.1f }, weaponDetail);
        DrawCubeV({ 0.55f, 0.04f, 0.0f }, { 0.5f, 0.04f, 0.04f }, weaponDetail);
        DrawCubeV({ -0.6f, 0.0f, 0.0f }, { 0.4f, 0.08f, 0.1f }, weaponColor);
        rlPopMatrix(); // End Weapon
    }
    rlPopMatrix(); // End Right Arm

    rlPushMatrix();
    rlTranslatef(-0.3f, 0.75f + torsoYOffset, 0.2f);
    rlRotatef(torsoXRot, 1.0, 0, 0);
    rlRotatef(leftArmRotZ, 0.0f, 0.0f, 1.0f);
    rlRotatef(leftArmRotX, 1.0f, 0.0f, 0.0f);
    DrawCubeV({ 0, -0.35f, 0 }, { 0.2f, 0.7f, 0.2f }, colors.shirt);
    rlPopMatrix(); // End Left Arm

    rlPopMatrix(); // End Character
}

// Helper to get enemy position
Vector3 GetEnemyPosition(const Enemy& enemy) {
    btVector3 pos = enemy.body->getWorldTransform().getOrigin();
    return { pos.x(), pos.y(), pos.z() };
}


int main() {
    // --- Initialization ---
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Uncharted Blockout - Editor & NPC Update");
    SetTargetFPS(60);

    GameState gameState = STATE_GAMEPLAY;
    GameSettings settings;
    DisableCursor();

    // --- Camera Setup ---
    Camera3D playerCamera = { 0 };
    playerCamera.up = { 0.0f, 1.0f, 0.0f };
    playerCamera.fovy = 60.0f;
    playerCamera.projection = CAMERA_PERSPECTIVE;
    float cameraDistance = 8.0f;
    float cameraDistanceTarget = 8.0f;

    Camera3D editorCamera = { 0 };
    editorCamera.position = { 0.0f, 15.0f, 15.0f };
    editorCamera.target = { 0.0f, 0.0f, 0.0f };
    editorCamera.up = { 0.0f, 1.0f, 0.0f };
    editorCamera.fovy = 60.0f;
    editorCamera.projection = CAMERA_PERSPECTIVE;
    float editorYaw = -90.0f * DEG2RAD;
    float editorPitch = -45.0f * DEG2RAD;


    float playerYaw = -90.0f * DEG2RAD;
    float playerPitch = 20.0f * DEG2RAD;
    float sensitivity = 0.003f;
    float playerHeight = 2.0f;
    float animationTimer = 0.0f;
    float playerDeathTimer = 0.0f;

    // --- Character Colors ---
    CharacterColors playerColors = { { 240, 220, 190, 255 }, { 50, 60, 180, 255 }, { 70, 80, 90, 255 } };
    CharacterColors enemyColors = { { 200, 180, 150, 255 }, { 180, 60, 50, 255 }, { 90, 80, 70, 255 } };

    // --- Bullet Physics Setup ---
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -25.0f, 0));

    // --- World Creation ---
    std::list<PhysicsBlock> worldBlocks;
    std::list<Enemy> enemies;
    CreateShooterArena(dynamicsWorld, worldBlocks, enemies);
    std::vector<VisualEffect> vfx;
    float enemyRespawnTimer = 0.0f;
    const float ENEMY_RESPAWN_TIME = 5.0f;

    // --- Player Setup ---
    Player player = {};
    btCollisionShape* playerShape = new btCapsuleShape(0.4f, playerHeight - 0.8f);
    btDefaultMotionState* playerMotion = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 5, 0)));
    btScalar playerMass = 1.0f;
    btVector3 playerInertia(0, 0, 0);
    playerShape->calculateLocalInertia(playerMass, playerInertia);
    btRigidBody::btRigidBodyConstructionInfo playerCI(playerMass, playerMotion, playerShape, playerInertia);
    player.body = new btRigidBody(playerCI);
    player.body->setAngularFactor(0);
    player.body->setActivationState(DISABLE_DEACTIVATION);
    player.body->setFriction(0.0f); // Set to 0 to prevent sticking to walls when jumping
    player.body->setUserIndex(1); // Use 1 for player
    player.maxHealth = 100;
    player.health = player.maxHealth;
    player.state = STATE_ON_GROUND;
    dynamicsWorld->addRigidBody(player.body);

    // --- Weapon State ---
    bool isAiming = false;
    const int magazineSize = 30;
    int currentAmmo = magazineSize;
    float reloadTimer = 0.0f;
    const float reloadTime = 2.0f;
    float fireTimer = 0.0f;
    const float fireRate = 0.1f;

    // --- Editor State ---
    Vector3 editorCursorPos = { 0 };
    bool editorCursorValid = false;

    // --- Dialogue System State ---
    bool inDialogue = false;
    Enemy* dialoguePartner = nullptr;
    std::string currentDialogue = "";
    std::vector<std::string> npcDialogueLines = {
        "Hello there, traveller.",
        "The weather is strange today, isn't it?",
        "Be careful, I've heard strange noises coming from the ruins.",
        "Are you looking for something?",
        "I'm just admiring the view. It never gets old.",
        "Sometimes I wonder what lies beyond those walls."
    };

    // --- Main Game Loop ---
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        animationTimer += dt;

        // --- State Switching ---
        if (IsKeyPressed(KEY_TAB)) {
            if (gameState == STATE_SETTINGS) {
                if (settings.inEditMode) {
                    gameState = STATE_EDITOR;
                    editorCamera.position = playerCamera.position;
                    editorYaw = playerYaw;
                    editorPitch = playerPitch;
                    DisableCursor();
                }
                else {
                    gameState = STATE_GAMEPLAY;
                    DisableCursor();
                }
            }
            else { // From Gameplay or Editor
                gameState = STATE_SETTINGS;
                EnableCursor();
            }
        }

        // --- Player Position ---
        btVector3 playerPosBt = player.body->getWorldTransform().getOrigin();
        Vector3 playerPos = { playerPosBt.getX(), playerPosBt.getY(), playerPosBt.getZ() };

        // ==============================================================================
        // --- GAMEPLAY STATE ---
        // ==============================================================================
        if (gameState == STATE_GAMEPLAY) {
            // --- Player Camera Control ---
            if (player.state != STATE_DYING) {
                Vector2 mouseDelta = GetMouseDelta();
                playerYaw -= mouseDelta.x * sensitivity;
                playerPitch -= mouseDelta.y * sensitivity;
                if (playerPitch > 89.0f * DEG2RAD) playerPitch = 89.0f * DEG2RAD;
                if (playerPitch < -89.0f * DEG2RAD) playerPitch = -89.0f * DEG2RAD;
            }

            // --- Player Ground Check ---
            btVector3 start = playerPosBt;
            btVector3 end = start - btVector3(0, playerHeight * 0.55f, 0);
            btCollisionWorld::ClosestRayResultCallback groundRay(start, end);
            dynamicsWorld->rayTest(start, end, groundRay);
            bool onGround = groundRay.hasHit();

            if (onGround) {
                player.state = STATE_ON_GROUND;
            }

            // --- Handle Dialogue ---
            if (IsKeyPressed(KEY_E)) {
                if (inDialogue) {
                    inDialogue = false;
                    dialoguePartner = nullptr;
                }
                else if (!settings.enemiesAttack && player.state != STATE_DYING) {
                    for (auto& enemy : enemies) {
                        if (enemy.state == STATE_DYING) continue;
                        Vector3 enemyPos = GetEnemyPosition(enemy);
                        if (Vector3DistanceSqr(playerPos, enemyPos) < 16.0f) {
                            inDialogue = true;
                            dialoguePartner = &enemy;
                            currentDialogue = npcDialogueLines[GetRandomValue(0, npcDialogueLines.size() - 1)];

                            // Make them face each other
                            Vector3 toPlayer = Vector3Subtract(playerPos, enemyPos);
                            Vector3 toNpc = Vector3Scale(toPlayer, -1.0f);

                            dialoguePartner->modelRotationAngle = atan2f(toPlayer.x, toPlayer.z) * RAD2DEG + 180.0f;
                            player.modelRotationAngle = atan2f(toNpc.x, toNpc.z) * RAD2DEG + 180.0f;
                            break;
                        }
                    }
                }
            }

            // Freeze participants during dialogue
            if (inDialogue) {
                player.isMoving = false;
                player.body->setLinearVelocity({ 0, player.body->getLinearVelocity().y(), 0 });
                if (dialoguePartner) {
                    dialoguePartner->isMoving = false;
                    dialoguePartner->body->setLinearVelocity({ 0, dialoguePartner->body->getLinearVelocity().y(), 0 });
                }
            }


            // --- Handle Player Input & Movement ---
            if (player.state != STATE_DYING && !inDialogue) {
                float speed = 8.0f;
                Vector3 moveInput = { 0 };
                if (IsKeyDown(KEY_W)) moveInput.z = 1.0f;
                if (IsKeyDown(KEY_S)) moveInput.z = -1.0f;
                if (IsKeyDown(KEY_A)) moveInput.x = -1.0f;
                if (IsKeyDown(KEY_D)) moveInput.x = 1.0f;
                player.isMoving = (moveInput.x != 0 || moveInput.z != 0);

                Vector3 cameraForwardRaw = { sinf(playerYaw), 0, cosf(playerYaw) };
                Vector3 cameraRight = { -cosf(playerYaw), 0, sinf(playerYaw) };
                Vector3 playerForward = cameraForwardRaw;

                isAiming = IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && player.state != STATE_JUMPING && reloadTimer <= 0;

                if (isAiming) {
                    playerForward = cameraForwardRaw;
                    player.modelRotationAngle = atan2f(playerForward.x, playerForward.z) * RAD2DEG + 180.0f;
                }
                else if (player.isMoving) {
                    Vector3 worldMoveDir = Vector3Normalize(Vector3Add(Vector3Scale(cameraForwardRaw, moveInput.z), Vector3Scale(cameraRight, moveInput.x)));
                    playerForward = worldMoveDir;
                    player.modelRotationAngle = atan2f(playerForward.x, playerForward.z) * RAD2DEG + 180.0f;
                }

                Vector3 worldMove = Vector3Normalize(Vector3Add(Vector3Scale(cameraForwardRaw, moveInput.z), Vector3Scale(cameraRight, moveInput.x)));
                btVector3 currentVel = player.body->getLinearVelocity();
                btVector3 desiredVel(0, currentVel.getY(), 0);

                if (player.isMoving && (player.state == STATE_ON_GROUND || player.state == STATE_JUMPING)) {
                    desiredVel.setX(worldMove.x * speed);
                    desiredVel.setZ(worldMove.z * speed);
                }
                if (player.state == STATE_ON_GROUND || player.state == STATE_JUMPING) {
                    player.body->setLinearVelocity(desiredVel);
                }

                if (IsKeyPressed(KEY_SPACE) && player.state == STATE_ON_GROUND) {
                    player.body->applyCentralImpulse(btVector3(0, 12.0f, 0));
                    player.state = STATE_JUMPING;
                }

                if (fireTimer > 0) fireTimer -= dt;
                if (reloadTimer > 0) {
                    reloadTimer -= dt;
                    if (reloadTimer <= 0) {
                        currentAmmo = magazineSize;
                    }
                }
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && fireTimer <= 0 && reloadTimer <= 0 && player.state != STATE_JUMPING) {
                    if (currentAmmo > 0) {
                        if (!settings.playerInfiniteAmmo) currentAmmo--;
                        fireTimer = fireRate;

                        btVector3 rayStart = { playerCamera.position.x, playerCamera.position.y, playerCamera.position.z };
                        Vector3 camForward = Vector3Normalize(Vector3Subtract(playerCamera.target, playerCamera.position));
                        btVector3 rayEnd = rayStart + btVector3(camForward.x, camForward.y, camForward.z) * 1000.0f;
                        btCollisionWorld::ClosestRayResultCallback bulletRay(rayStart, rayEnd);
                        dynamicsWorld->rayTest(rayStart, rayEnd, bulletRay);

                        Vector3 tracerEnd;
                        if (bulletRay.hasHit()) {
                            btVector3 p = bulletRay.m_hitPointWorld;
                            tracerEnd = { p.x(), p.y(), p.z() };
                            vfx.push_back({ tracerEnd, tracerEnd, 0.1f, RED });

                            const btCollisionObject* hitObj = bulletRay.m_collisionObject;
                            if (hitObj->getUserIndex() == 2) {
                                Enemy* enemy = (Enemy*)hitObj->getUserPointer();
                                if (enemy->state != STATE_DYING) {
                                    enemy->health -= 25;
                                    enemy->body->applyCentralImpulse(btVector3(camForward.x, camForward.y, camForward.z) * 2.0f);
                                }
                            }
                        }
                        else {
                            tracerEnd = { rayEnd.x(), rayEnd.y(), rayEnd.z() };
                        }
                        Matrix rotMatrix = MatrixRotateY((player.modelRotationAngle - 180.0f) * DEG2RAD);
                        Vector3 rotatedOffset = Vector3Transform({ 0.0f, 1.15f, 0.7f }, rotMatrix);
                        Vector3 tracerStart = Vector3Add(playerPos, rotatedOffset);
                        vfx.push_back({ tracerStart, tracerEnd, 0.05f, YELLOW });
                    }
                    else {
                        if (reloadTimer <= 0) reloadTimer = reloadTime;
                    }
                }
                if (IsKeyPressed(KEY_R) && reloadTimer <= 0 && currentAmmo < magazineSize && !settings.playerInfiniteAmmo) {
                    reloadTimer = reloadTime;
                }
            }
            else if (player.state == STATE_DYING) { // Player is dying
                player.body->setLinearVelocity({ 0,0,0 });
                playerDeathTimer += dt;
                if (playerDeathTimer > 3.0f) {
                    btTransform trans;
                    trans.setIdentity();
                    trans.setOrigin(btVector3(0, 5, 0));
                    player.body->setWorldTransform(trans);
                    player.body->clearForces();
                    player.body->setLinearVelocity({ 0,0,0 });
                    player.health = player.maxHealth;
                    player.state = STATE_ON_GROUND;
                    currentAmmo = magazineSize;
                    playerDeathTimer = 0.0f;
                }
            }

            int deadEnemies = 0;
            for (auto& enemy : enemies) {
                if (enemy.state == STATE_DYING) {
                    deadEnemies++;
                    enemy.deathTimer += dt;
                    enemy.isMoving = false;
                    enemy.body->setLinearVelocity({ 0,0,0 });
                    continue;
                }
                if (enemy.health <= 0) {
                    enemy.state = STATE_DYING;
                    enemy.body->setCollisionFlags(enemy.body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
                    continue;
                }

                // Don't update AI if in dialogue with this enemy
                if (inDialogue && &enemy == dialoguePartner) continue;

                if (!settings.enemiesFrozen) {
                    enemy.animationTimer += dt;
                    float enemySpeed = settings.enemiesSuperSpeed ? 8.0f : 4.0f;
                    if (settings.enemiesAttack && player.state != STATE_DYING) {
                        Vector3 enemyPos = GetEnemyPosition(enemy);
                        Vector3 toPlayer = Vector3Subtract(playerPos, enemyPos);
                        float distance = Vector3Length(toPlayer);
                        if (distance < 1.5f) {
                            if (!settings.playerInfiniteHealth) player.health -= 20 * dt;
                            enemy.isMoving = false;
                        }
                        else if (distance < 20.0f) {
                            Vector3 dir = Vector3Normalize(toPlayer);
                            btVector3 enemyVel = enemy.body->getLinearVelocity();
                            enemy.body->setLinearVelocity({ dir.x * enemySpeed, enemyVel.y(), dir.z * enemySpeed });
                            enemy.modelRotationAngle = atan2f(dir.x, dir.z) * RAD2DEG + 180.0f;
                            enemy.isMoving = true;
                        }
                        else {
                            enemy.isMoving = false;
                            enemy.body->setLinearVelocity({ 0, enemy.body->getLinearVelocity().y(), 0 });
                        }
                    }
                    else { // Wander behavior if not attacking
                        enemy.aiStateTimer -= dt;
                        if (enemy.aiStateTimer <= 0) {
                            enemy.isMoving = !enemy.isMoving;
                            enemy.aiStateTimer = (float)GetRandomValue(20, 50) / 10.0f;
                            if (enemy.isMoving) {
                                float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                                enemy.wanderDirection = { sinf(angle), 0, cosf(angle) };
                                enemy.modelRotationAngle = angle * RAD2DEG + 180.0f;
                            }
                        }
                        btVector3 enemyVel = enemy.body->getLinearVelocity();
                        if (enemy.isMoving) {
                            enemy.body->setLinearVelocity({ enemy.wanderDirection.x * enemySpeed, enemyVel.y(), enemy.wanderDirection.z * enemySpeed });
                        }
                        else {
                            enemy.body->setLinearVelocity({ 0, enemyVel.y(), 0 });
                        }
                    }
                }
                else {
                    enemy.body->setLinearVelocity({ 0, enemy.body->getLinearVelocity().y(), 0 });
                }
            }

            if (player.health <= 0 && player.state != STATE_DYING) {
                player.state = STATE_DYING;
                playerDeathTimer = 0.0f;
            }

            dynamicsWorld->stepSimulation(dt, 10);

            if (deadEnemies > 0) {
                enemyRespawnTimer += dt * deadEnemies;
                if (enemyRespawnTimer >= ENEMY_RESPAWN_TIME) {
                    enemyRespawnTimer = 0.0f;
                    if (!enemySpawnPoints.empty()) {
                        int spawnIdx = GetRandomValue(0, enemySpawnPoints.size() - 1);
                        CreateEnemy(dynamicsWorld, enemies, enemySpawnPoints[spawnIdx]);
                    }
                }
            }

            for (auto it = enemies.begin(); it != enemies.end(); ) {
                if (it->state == STATE_DYING && it->deathTimer > 3.0f) {
                    dynamicsWorld->removeRigidBody(it->body);
                    delete it->body->getMotionState();
                    delete it->body->getCollisionShape();
                    delete it->body;
                    it = enemies.erase(it);
                }
                else {
                    ++it;
                }
            }

        }
        else if (gameState == STATE_EDITOR) {
            Vector2 mouseDelta = GetMouseDelta();
            editorYaw -= mouseDelta.x * sensitivity;
            editorPitch -= mouseDelta.y * sensitivity;
            if (editorPitch > 89.0f * DEG2RAD) editorPitch = 89.0f * DEG2RAD;
            if (editorPitch < -89.0f * DEG2RAD) editorPitch = -89.0f * DEG2RAD;
            Vector3 camForward = { cosf(editorPitch) * sinf(editorYaw), sinf(editorPitch), cosf(editorPitch) * cosf(editorYaw) };
            Vector3 camRight = { -cosf(editorYaw), 0, sinf(editorYaw) };
            Vector3 moveDir = { 0 };
            float editorSpeed = 15.0f;
            if (IsKeyDown(KEY_W)) moveDir = Vector3Add(moveDir, camForward);
            if (IsKeyDown(KEY_S)) moveDir = Vector3Subtract(moveDir, camForward);
            if (IsKeyDown(KEY_A)) moveDir = Vector3Add(moveDir, camRight);
            if (IsKeyDown(KEY_D)) moveDir = Vector3Subtract(moveDir, camRight);
            if (IsKeyDown(KEY_SPACE)) moveDir.y += 1.0f;
            if (IsKeyDown(KEY_LEFT_CONTROL)) moveDir.y -= 1.0f;
            if (Vector3LengthSqr(moveDir) > 0) moveDir = Vector3Normalize(moveDir);
            editorCamera.position = Vector3Add(editorCamera.position, Vector3Scale(moveDir, editorSpeed * dt));
            editorCamera.target = Vector3Add(editorCamera.position, camForward);

            Ray editorRay = { editorCamera.position, Vector3Normalize(Vector3Subtract(editorCamera.target, editorCamera.position)) };
            btVector3 rayFrom(editorRay.position.x, editorRay.position.y, editorRay.position.z);
            btVector3 rayTo(editorRay.position.x + editorRay.direction.x * 1000.0f, editorRay.position.y + editorRay.direction.y * 1000.0f, editorRay.position.z + editorRay.direction.z * 1000.0f);
            btCollisionWorld::ClosestRayResultCallback editorRayCallback(rayFrom, rayTo);
            dynamicsWorld->rayTest(rayFrom, rayTo, editorRayCallback);
            editorCursorValid = editorRayCallback.hasHit();

            if (editorCursorValid) {
                btVector3 p = editorRayCallback.m_hitPointWorld;
                btVector3 n = editorRayCallback.m_hitNormalWorld;
                editorCursorPos = { p.x() + n.x() * 0.01f, p.y() + n.y() * 0.01f, p.z() + n.z() * 0.01f }; // Offset slightly to avoid z-fighting

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    switch (settings.editorMode) {
                    case MODE_PLACE:
                        CreateStaticBox(dynamicsWorld, worldBlocks, editorCursorPos, settings.editorBlockSize, settings.editorBlockColor);
                        break;
                    case MODE_SPAWN:
                        enemySpawnPoints.push_back(editorCursorPos);
                        break;
                    case MODE_ERASE: {
                        const btCollisionObject* hitObj = editorRayCallback.m_collisionObject;
                        if (hitObj->getUserIndex() == -1) { // It's a world block
                            PhysicsBlock* block = (PhysicsBlock*)hitObj->getUserPointer();
                            for (auto it = worldBlocks.begin(); it != worldBlocks.end(); ++it) {
                                if (it->body == block->body) {
                                    dynamicsWorld->removeRigidBody(it->body);
                                    delete it->body->getMotionState();
                                    delete it->body->getCollisionShape();
                                    delete it->body;
                                    worldBlocks.erase(it);
                                    break;
                                }
                            }
                        }
                        // Also check for spawn points to erase
                        for (size_t i = 0; i < enemySpawnPoints.size(); ++i) {
                            if (Vector3DistanceSqr(editorCursorPos, enemySpawnPoints[i]) < 2.0f) {
                                enemySpawnPoints.erase(enemySpawnPoints.begin() + i);
                                break;
                            }
                        }
                    } break;
                    }
                }
            }
        }

        for (int i = vfx.size() - 1; i >= 0; --i) {
            vfx[i].life -= dt;
            if (vfx[i].life <= 0) {
                vfx.erase(vfx.begin() + i);
            }
        }

        Camera3D& currentCamera = (gameState == STATE_EDITOR) ? editorCamera : playerCamera;

        if (gameState == STATE_GAMEPLAY) {
            Vector3 focusPos = Vector3Add(playerPos, { 0, 1.5f, 0 });
            cameraDistanceTarget = isAiming ? 3.0f : 8.0f;
            playerCamera.target = focusPos;
            cameraDistance = Lerp(cameraDistance, cameraDistanceTarget, dt * 5.0f);
            float idealCamX = focusPos.x - (cosf(playerPitch) * sinf(playerYaw) * cameraDistance);
            float idealCamY = focusPos.y - (sinf(playerPitch) * cameraDistance);
            float idealCamZ = focusPos.z - (cosf(playerPitch) * cosf(playerYaw) * cameraDistance);
            btVector3 camRayStart(focusPos.x, focusPos.y, focusPos.z);
            btVector3 camRayEnd(idealCamX, idealCamY, idealCamZ);
            btCollisionWorld::ClosestRayResultCallback camRay(camRayStart, camRayEnd);
            dynamicsWorld->rayTest(camRayStart, camRayEnd, camRay);
            if (camRay.hasHit()) {
                btVector3 hitPoint = camRay.m_hitPointWorld;
                Vector3 diff = Vector3Subtract({ idealCamX, idealCamY, idealCamZ }, { hitPoint.getX(), hitPoint.getY(), hitPoint.getZ() });
                Vector3 norm = Vector3Normalize(diff);
                playerCamera.position = Vector3Add({ hitPoint.getX(), hitPoint.getY(), hitPoint.getZ() }, Vector3Scale(norm, 0.2f));
            }
            else {
                playerCamera.position = { idealCamX, idealCamY, idealCamZ };
            }
        }

        BeginDrawing();
        ClearBackground(SKYBLUE);
        BeginMode3D(currentCamera);

        for (const auto& block : worldBlocks) {
            btTransform trans;
            block.body->getMotionState()->getWorldTransform(trans);
            btVector3 pos = trans.getOrigin();
            DrawCubeV({ pos.getX(), pos.getY(), pos.getZ() }, block.size, block.color);
            DrawCubeWiresV({ pos.getX(), pos.getY(), pos.getZ() }, block.size, BLACK);
        }

        for (const auto& sp : enemySpawnPoints) {
            DrawCylinder(sp, 0.5f, 0.5f, 0.2f, 16, Fade(PURPLE, 0.5f));
            DrawCylinderWires(sp, 0.5f, 0.5f, 0.2f, 16, PURPLE);
        }

        for (const auto& enemy : enemies) {
            Vector3 enemyPos = GetEnemyPosition(enemy);
            bool isThisNPCtalking = (inDialogue && &enemy == dialoguePartner);
            DrawAnimatedCharacter(enemyPos, enemy.modelRotationAngle, enemy.state, enemy.animationTimer, enemy.isMoving, false, isThisNPCtalking, 0, 0, enemyColors);
        }

        for (const auto& effect : vfx) {
            if (Vector3Equals(effect.start, effect.end)) {
                DrawSphere(effect.start, 0.2f, effect.color);
            }
            else {
                DrawLine3D(effect.start, effect.end, effect.color);
            }
        }

        DrawAnimatedCharacter(playerPos, player.modelRotationAngle, player.state, animationTimer, player.isMoving, isAiming, inDialogue, reloadTimer, fireTimer > 0 ? (fireTimer / fireRate) : 0.0f, playerColors);

        if (gameState == STATE_EDITOR && editorCursorValid) {
            switch (settings.editorMode) {
            case MODE_PLACE:
                DrawCube(editorCursorPos, settings.editorBlockSize.x, settings.editorBlockSize.y, settings.editorBlockSize.z, Fade(LIME, 0.5f));
                DrawCubeWires(editorCursorPos, settings.editorBlockSize.x, settings.editorBlockSize.y, settings.editorBlockSize.z, LIME);
                break;
            case MODE_ERASE:
                DrawSphere(editorCursorPos, 0.5f, Fade(RED, 0.5f));
                DrawSphereWires(editorCursorPos, 0.5f, 10, 10, RED);
                break;
            case MODE_SPAWN:
                DrawCylinder(editorCursorPos, 0.5f, 0.5f, 0.2f, 16, Fade(PURPLE, 0.5f));
                DrawCylinderWires(editorCursorPos, 0.5f, 0.5f, 0.2f, 16, PURPLE);
                break;
            }
        }

        EndMode3D();

        for (const auto& enemy : enemies) {
            if (enemy.state != STATE_DYING) {
                btVector3 enemyPosBt = enemy.body->getWorldTransform().getOrigin();
                Vector3 enemyWorldPos = { enemyPosBt.x(), enemyPosBt.y() + 2.4f, enemyPosBt.z() };
                Vector3 camToEnemy = Vector3Subtract(enemyWorldPos, currentCamera.position);
                Vector3 camForward = Vector3Normalize(Vector3Subtract(currentCamera.target, currentCamera.position));
                if (Vector3DotProduct(camToEnemy, camForward) > 0) {
                    Vector2 healthBarPos = GetWorldToScreen(enemyWorldPos, currentCamera);
                    float healthPercent = (float)enemy.health / (float)enemy.maxHealth;
                    DrawRectangle(healthBarPos.x - 25, healthBarPos.y, 50, 6, Fade(BLACK, 0.5f));
                    DrawRectangle(healthBarPos.x - 25, healthBarPos.y, 50 * healthPercent, 6, LIME);
                    DrawRectangleLines(healthBarPos.x - 25, healthBarPos.y, 50, 6, DARKGRAY);
                }
            }
        }

        if (gameState == STATE_GAMEPLAY) {
            float healthPercent = (float)player.health / (float)player.maxHealth;
            DrawRectangle(20, screenHeight - 40, 200, 20, Fade(BLACK, 0.5f));
            DrawRectangle(20, screenHeight - 40, 200 * healthPercent, 20, RED);
            DrawRectangleLines(20, screenHeight - 40, 200, 20, DARKGRAY);
            std::string ammoText = (settings.playerInfiniteAmmo) ? "INF / INF" : TextFormat("%d / %d", currentAmmo, magazineSize);
            if (reloadTimer > 0) ammoText = "RELOADING...";
            DrawText(ammoText.c_str(), screenWidth - 150, screenHeight - 40, 30, DARKGRAY);
            if (isAiming) DrawRectangle(screenWidth / 2 - 2, screenHeight / 2 - 2, 4, 4, RED);

            if (!settings.enemiesAttack) {
                DrawText("Attack mode is OFF. Press 'E' near characters to talk.", 20, screenHeight - 70, 20, WHITE);
            }

            if (inDialogue && dialoguePartner) {
                DrawRectangle(10, screenHeight - 120, screenWidth - 20, 110, Fade(BLACK, 0.7f));
                DrawText(TextFormat("NPC says: \"%s\"", currentDialogue.c_str()), 25, screenHeight - 105, 20, WHITE);
                DrawText("Press [E] to continue...", screenWidth - 220, screenHeight - 40, 20, GRAY);
            }
        }

        if (gameState == STATE_SETTINGS) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));
            int menuX = 50;
            int menuY = 100;
            int spacing = 30;
            DrawText("Settings (Tab to close)", menuX, menuY - 40, 20, WHITE);
            auto DrawCheckbox = [&](int x, int y, const char* text, bool& value) {
                Rectangle box = { (float)x, (float)y, 20, 20 };
                DrawRectangleLinesEx(box, 2, value ? LIME : WHITE);
                if (value) DrawRectangle(x + 4, y + 4, 12, 12, LIME);
                DrawText(text, x + 30, y + 2, 20, WHITE);
                if (CheckCollisionPointRec(GetMousePosition(), box) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    value = !value;
                }
                };
            auto DrawButton = [&](int x, int y, int w, int h, const char* text, Color color, bool active = false) -> bool {
                Rectangle btn = { (float)x, (float)y, (float)w, (float)h };
                bool clicked = false;
                Color baseColor = active ? LIME : color;
                if (CheckCollisionPointRec(GetMousePosition(), btn)) {
                    DrawRectangleRec(btn, Fade(baseColor, 0.8f));
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) clicked = true;
                }
                else {
                    DrawRectangleRec(btn, baseColor);
                }
                DrawRectangleLinesEx(btn, 2, WHITE);
                DrawText(text, x + w / 2 - MeasureText(text, 20) / 2, y + h / 2 - 10, 20, WHITE);
                return clicked;
                };
            auto DrawFloatSpinner = [&](int x, int y, const char* label, float& value, float step) {
                DrawText(label, x, y + 2, 20, WHITE);
                DrawText(TextFormat("%.1f", value), x + 100, y + 2, 20, WHITE);
                if (DrawButton(x + 160, y, 25, 25, "-", MAROON)) value -= step;
                if (DrawButton(x + 195, y, 25, 25, "+", DARKGREEN)) value += step;
                if (value < 0.1f) value = 0.1f;
                };
            DrawText("-- Enemy AI --", menuX, menuY, 20, YELLOW);
            menuY += spacing;
            DrawCheckbox(menuX, menuY, "Freeze Enemies", settings.enemiesFrozen);
            menuY += spacing;
            DrawCheckbox(menuX, menuY, "Attack Player", settings.enemiesAttack);
            menuY += spacing;
            DrawCheckbox(menuX, menuY, "Super Speed Enemies", settings.enemiesSuperSpeed);
            menuY += spacing * 2;
            DrawText("-- Player Settings --", menuX, menuY, 20, SKYBLUE);
            menuY += spacing;
            DrawCheckbox(menuX, menuY, "Infinite Health", settings.playerInfiniteHealth);
            menuY += spacing;
            DrawCheckbox(menuX, menuY, "Infinite Ammo", settings.playerInfiniteAmmo);
            int editorMenuX = screenWidth / 2;
            int editorMenuY = 100;
            DrawText("-- World Editor --", editorMenuX, editorMenuY, 20, ORANGE);
            editorMenuY += spacing;
            DrawCheckbox(editorMenuX, editorMenuY, "Edit Mode", settings.inEditMode);
            if (settings.inEditMode) {
                DrawText("Exit menu to enter Edit Mode", editorMenuX + 30, editorMenuY + spacing, 10, RAYWHITE);
                editorMenuY += spacing * 1.5f;

                DrawText("Editor Tool:", editorMenuX, editorMenuY, 20, RAYWHITE);
                editorMenuY += spacing;
                if (DrawButton(editorMenuX, editorMenuY, 80, 25, "Place", DARKGRAY, settings.editorMode == MODE_PLACE)) settings.editorMode = MODE_PLACE;
                if (DrawButton(editorMenuX + 90, editorMenuY, 80, 25, "Erase", MAROON, settings.editorMode == MODE_ERASE)) settings.editorMode = MODE_ERASE;
                if (DrawButton(editorMenuX + 180, editorMenuY, 80, 25, "Spawn", PURPLE, settings.editorMode == MODE_SPAWN)) settings.editorMode = MODE_SPAWN;
                editorMenuY += spacing * 1.5f;


                if (settings.editorMode == MODE_PLACE) {
                    DrawText("Block Settings:", editorMenuX, editorMenuY, 20, RAYWHITE);
                    editorMenuY += spacing;
                    DrawFloatSpinner(editorMenuX, editorMenuY, "Size X:", settings.editorBlockSize.x, 0.5f);
                    editorMenuY += spacing;
                    DrawFloatSpinner(editorMenuX, editorMenuY, "Size Y:", settings.editorBlockSize.y, 0.5f);
                    editorMenuY += spacing;
                    DrawFloatSpinner(editorMenuX, editorMenuY, "Size Z:", settings.editorBlockSize.z, 0.5f);
                    editorMenuY += spacing * 1.5f;
                    DrawText("Block Color:", editorMenuX, editorMenuY, 20, RAYWHITE);
                    editorMenuY += spacing;
                    int colorButtonX = editorMenuX;
                    int colorButtonY = editorMenuY;
                    Color colors[] = { ORANGE, RED, LIME, BLUE, PURPLE, GOLD, GRAY, DARKGRAY };
                    for (int i = 0; i < 8; ++i) {
                        Rectangle swatch = { (float)colorButtonX, (float)colorButtonY, 30, 30 };
                        DrawRectangleRec(swatch, colors[i]);
                        if (CheckCollisionPointRec(GetMousePosition(), swatch)) {
                            DrawRectangleLinesEx(swatch, 3, WHITE);
                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) settings.editorBlockColor = colors[i];
                        }
                        colorButtonX += 40;
                    }
                }
            }
        }

        if (gameState == STATE_EDITOR) {
            std::string modeText = "MODE: ";
            switch (settings.editorMode) {
            case MODE_PLACE: modeText += "PLACE"; break;
            case MODE_ERASE: modeText += "ERASE"; break;
            case MODE_SPAWN: modeText += "SPAWN"; break;
            }
            DrawText("EDIT MODE | Tab to return to settings", 10, 40, 20, YELLOW);
            DrawText("[LMB] Use Tool", 10, 65, 20, YELLOW);
            DrawText(modeText.c_str(), 10, 90, 20, YELLOW);
            DrawRectangle(screenWidth / 2 - 2, screenHeight / 2 - 2, 4, 4, WHITE);
        }

        DrawFPS(10, 10);
        EndDrawing();
    }

    // --- Cleanup ---
    for (auto& block : worldBlocks) {
        dynamicsWorld->removeRigidBody(block.body);
        delete block.body->getMotionState();
        delete block.body->getCollisionShape();
        delete block.body;
    }
    for (auto& enemy : enemies) {
        dynamicsWorld->removeRigidBody(enemy.body);
        delete enemy.body->getMotionState();
        delete enemy.body->getCollisionShape();
        delete enemy.body;
    }

    dynamicsWorld->removeRigidBody(player.body);
    delete player.body->getMotionState();
    delete player.body;
    delete playerShape;

    delete dynamicsWorld;
    delete solver;
    delete overlappingPairCache;
    delete dispatcher;
    delete collisionConfiguration;

    CloseWindow();
    return 0;
}

