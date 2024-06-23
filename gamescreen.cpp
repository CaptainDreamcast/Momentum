#include "gamescreen.h"
#include "storyscreen.h"

static struct 
{
    int mLevel = 0;
} gGameScreenData;

class GameScreen
{
    public:
    GameScreen() {
        load();
    }

    MugenSpriteFile mSprites;
    MugenAnimations mAnimations;
    MugenSounds mSounds;

    void loadFiles() {
        mSprites = loadMugenSpriteFileWithoutPalette("game/GAME.sff");
        mAnimations = loadMugenAnimationFile("game/GAME.air");
        mSounds = loadMugenSoundFile("game/GAME.snd");
    }

    void load() {
        loadFiles();
        loadBG();
        loadCar();
        loadSlingshot();
        loadRoad();
        loadObstacles();
        loadGoal();
        loadUI();

        streamMusicFile("game/GAME.ogg");
    }

    void update() {
        updateBG();
        updateCar();
        updateSlingshot();
        updateRoad();
        updateObstacles();
        updateGoal();
        updateUI();
    }


    // BG
    int bgEntityId;
    void loadBG() {
        bgEntityId = addBlitzEntity(Vector3D(0, 0, 1));
        addBlitzMugenAnimationComponent(bgEntityId, &mSprites, &mAnimations, 1);
    }
    void updateBG() {
    }

    // Car
    int carEntityId;
    Vector2D carStartPosition = Vector2D(22, 210);
    Vector2D slingShotAxisPosition = Vector2D(42, 205);
    Vector2D carVelocity = Vector2D(0, 0);
    bool wasCarSlingShotted = false;
    bool isCarOnGround = false;
    void loadCar() {
        carEntityId = addBlitzEntity(carStartPosition.xyz(40));
        addBlitzMugenAnimationComponent(carEntityId, &mSprites, &mAnimations, 11);
    }
    void updateCar() {
        updateCarSlingShotStatus();
        updateCarFlying();
        updateCarHittingObstacles();
        updateCarLanding();
        updateCarOnGround();
    }

    void updateCarHittingObstacles()
    {
        if(!wasCarSlingShotted) return;
        if(isCarOnGround) return;

        for (int i = 0; i < obstacleEntityIds.size(); i++) {
            auto obstaclePosition = getBlitzEntityPosition(obstacleEntityIds[i]);
            GeoRectangle2D obstacleRectangle = GeoRectangle2D(obstaclePosition.x, obstaclePosition.y, 16, 16);
            auto carPosition = getBlitzEntityPosition(carEntityId);
            if (checkPointInRectangle(obstacleRectangle, carPosition.xy()) || checkPointInRectangle(obstacleRectangle, carPosition.xy() - Vector2D(0, 4)))
            {
                if(carVelocity.x > 0)
                {
                    tryPlayMugenSound(&mSounds, 1, 1);
                }
                carVelocity.x = 0;
                break;
            }
        }
    }

    void updateCarSlingShotStatus() {
        if(wasCarSlingShotted) return;

        if (wasSlingShotReleased()) {
            carVelocity = (slingShotAxisPosition - getBlitzEntityPosition(carEntityId)).xy() * 0.1;
            wasCarSlingShotted = true;
        }
    }

    void updateCarFlying() {
        if (wasCarSlingShotted && !isCarOnGround) {
            carVelocity.y += 1 / 60.0;
            setBlitzEntityPosition(carEntityId, getBlitzEntityPosition(carEntityId) + carVelocity.xyz(0));
        }
    }

    void updateCarLanding() {
        if (isCarOnGround || !wasCarSlingShotted) return;

        // check against all road parts if we are on the ground now
        for (int i = 0; i < roadEntityIds.size(); i++) {
            auto roadPosition = getBlitzEntityPosition(roadEntityIds[i]);
            GeoRectangle2D roadRectangle = GeoRectangle2D(roadPosition.x, roadPosition.y, 16, 4);
            auto carPosition = getBlitzEntityPosition(carEntityId);
            if (checkPointInRectangle(roadRectangle, carPosition.xy())) {
                carPosition.y = roadRectangle.mTopLeft.y -4;
                setBlitzEntityPosition(carEntityId, carPosition);
                tryPlayMugenSound(&mSounds, 1, 0);
                isCarOnGround = true;
                carVelocity.y = 0;
                break;
            }
        }
    }

    void updateCarOnGround() {
        if(!wasCarSlingShotted) return;
        if(!isCarOnGround) return;

        setBlitzEntityPosition(carEntityId, getBlitzEntityPosition(carEntityId) + carVelocity.xyz(0));
        carVelocity.x *= 0.98;

        updateCarAppearanceOnGround();
    }

    void updateCarAppearanceOnGround() {
        if(carVelocity.x > 0.001)
        {
            changeBlitzMugenAnimationIfDifferent(carEntityId, 10);
        }
        else{
            changeBlitzMugenAnimationIfDifferent(carEntityId, 11);
        }
    }





    // Slingshot
    int slingshotEntityId;
    int slingShotRubberId1;
    int slingShotRubberId2;
    void loadSlingshot() {
        slingshotEntityId = addBlitzEntity(Vector3D(42, 240, 30));
        addBlitzMugenAnimationComponent(slingshotEntityId, &mSprites, &mAnimations, 20);
        slingShotRubberId1 = addBlitzEntity(Vector3D(32, 205, 29));
        addBlitzMugenAnimationComponent(slingShotRubberId1, &mSprites, &mAnimations, 26);
        slingShotRubberId2 = addBlitzEntity(Vector3D(52, 205, 28));
        addBlitzMugenAnimationComponent(slingShotRubberId2, &mSprites, &mAnimations, 25);

        slingShotActualPosition = getBlitzEntityPosition(carEntityId).xy();
        updateSlingShotAppearanceWhileSlinging();
    }

    bool wasSlingShotPressed = false;
    bool wasSlingShotReleasedV = false;
    bool wasSlingShotReleasedFlankTriggered = false;
    Vector2D slingPointerPosition = Vector2D(0, 0);
    Vector2D slingShotActualPosition = Vector2D(0, 0);
    void updateSlingshot() {
        updateSlingShotStartSlinging();
        updateSlingShotWhileSlinging();
        updateSlingShotEndSlinging();
        updateSlingShotReturningToStartPositionAfterSling();
    }

    void updateSlingShotStartSlinging() {
        if(wasSlingShotPressed) return;

        if(hasPressedMouseLeft())
        {
            wasSlingShotPressed = true;
            slingPointerPosition = getMousePointerPosition();
        }
    }

    void updateSlingShotWhileSlinging() {
        if(!wasSlingShotPressed) return;
        if(wasSlingShotReleasedV) return;

        Vector2D pointerPosition = getMousePointerPosition();
        Vector2D delta = pointerPosition - slingPointerPosition;
        setBlitzEntityPosition(carEntityId, getBlitzEntityPosition(carEntityId) + delta.xyz(0));
        slingPointerPosition = pointerPosition;
        // make sure car is no more than 20 units away from start position
        {
            auto carPosition = getBlitzEntityPosition(carEntityId);
            auto delta = carPosition.xy() - carStartPosition;
            auto length = vecLength(delta);
            if(length > 50)
            {
                carPosition = (carStartPosition + delta / length * 50).xyz(carPosition.z);
                setBlitzEntityPosition(carEntityId, carPosition);
            }
        
        }

        slingShotActualPosition = getBlitzEntityPosition(carEntityId).xy();
        updateSlingShotAppearanceWhileSlinging();
    }

    void updateSlingShotAppearanceWhileSlinging() {
        auto carPosition = slingShotActualPosition;
        auto slingShotPosition = getBlitzEntityPosition(slingShotRubberId1);
        auto slingShotPosition2 = getBlitzEntityPosition(slingShotRubberId2);
        auto delta = carPosition - slingShotPosition.xy();
        auto delta2 = carPosition - slingShotPosition2.xy();
        auto length = vecLength(delta);
        auto length2 = vecLength(delta2);
        auto angle = getAngleFromDirection(delta);
        auto angle2 = getAngleFromDirection(delta2);
        setBlitzMugenAnimationBaseDrawScale(slingShotRubberId1, length / 25.0);
        setBlitzMugenAnimationAngle(slingShotRubberId1, angle);
        setBlitzMugenAnimationBaseDrawScale(slingShotRubberId2, length2 / 25.0);
        setBlitzMugenAnimationAngle(slingShotRubberId2, angle2);
    }

    void updateSlingShotReturningToStartPositionAfterSling() {
        if(!wasSlingShotReleasedV) return;
        if(!wasSlingShotReleasedFlankTriggered) return;

        auto relaxedPosition = carStartPosition;
        auto& currentPosition = slingShotActualPosition;

        auto delta = relaxedPosition - currentPosition;
        auto length = vecLength(delta);

        if(length < 2) {
            return;
        }

        currentPosition += delta / length * 2;

        updateSlingShotAppearanceWhileSlinging();
    }

    void updateSlingShotEndSlinging() {
        if(!wasSlingShotPressed) return;
        if(wasSlingShotReleasedV) return;

        if(!hasPressedMouseLeft())
        {
            tryPlayMugenSound(&mSounds, 1, 2);
            slingShotActualPosition = getBlitzEntityPosition(carEntityId).xy();
            wasSlingShotReleasedV = true;
        }
    }

    bool wasSlingShotReleased() {
        if(!wasSlingShotReleasedV) return false;
        if(wasSlingShotReleasedFlankTriggered) return false;
        wasSlingShotReleasedFlankTriggered = true;
        return true;
    }

    // Road
    std::vector<int> roadEntityIds;
    void loadRoad() {
        placeRoadAtPosition(Vector2D(178, 206), 20);
    }

    void placeRoadAtPosition(Vector2D pos, int count)
    {
        for (int i = 0; i < count; i++) {
            int roadEntityId = addBlitzEntity(pos.xyz(10));
            addBlitzMugenAnimationComponent(roadEntityId, &mSprites, &mAnimations, 30);
            roadEntityIds.push_back(roadEntityId);
            pos.x += 16;
        }
    }

    void updateRoad() {
    }

    // Obstacles
    std::vector<int> obstacleEntityIds;
    void loadObstacles() {
        placeObstaclesAtPosition(Vector2D(130, 0), 8);
    }

    void placeObstaclesAtPosition(Vector2D pos, int count)
    {
        for (int i = 0; i < count; i++) {
            int obstacleEntityId = addBlitzEntity(pos.xyz(20));
            addBlitzMugenAnimationComponent(obstacleEntityId, &mSprites, &mAnimations, 40);
            obstacleEntityIds.push_back(obstacleEntityId);
            pos.y += 16;
        }
    }

    void updateObstacles() {
    }

    // Goal
    int goalEntityId;
    void loadGoal() {
        goalEntityId = addBlitzEntity(Vector3D(310, 206, 9));
        addBlitzMugenAnimationComponent(goalEntityId, &mSprites, &mAnimations, 50);
    }
    void updateGoal() {
    }

    // UI
    void loadUI() {
        loadStart();
        loadWin();
        loadLose();
    }

    void updateUI() {
        updateStart();
        updateWin();
        updateLose();
    }

    // Start
    int startEntityId;
    bool isStartShown = true;
    int startTicks = 0;
    void loadStart() {
        startEntityId = addBlitzEntity(Vector3D(0, 0, 100));
        addBlitzMugenAnimationComponent(startEntityId, &mSprites, &mAnimations, 60);
    }

    void updateStart() {
        updateStartEnd();
    }

    void updateStartEnd() {
        if(!isStartShown) return;

        startTicks++;
        if(startTicks >= 60)
        {
            removeBlitzEntity(startEntityId);
            isStartShown = false;
        }
    }

    // Win
    int winEntityId;
    bool isWinShown = false;
    int winTicks = 0;

    void loadWin() {
        winEntityId = addBlitzEntity(Vector3D(0, 0, 100));
        addBlitzMugenAnimationComponent(winEntityId, &mSprites, &mAnimations, 62);
        setBlitzMugenAnimationVisibility(winEntityId, 0);
    }

    void updateWin() {
        if(isLoseShown) return;
        updateWinStart();
        updateWinEnd();
    }

    void updateWinStart() {
        if(isWinShown) return;

        auto carPosition = getBlitzEntityPosition(carEntityId);
        auto goalPosition = getBlitzEntityPosition(goalEntityId);
        if(vecLength(carPosition.xy() - goalPosition.xy()) < 20)
        {
            setBlitzMugenAnimationVisibility(winEntityId, 1);
            stopStreamingMusicFile();
            tryPlayMugenSound(&mSounds, 2, 1);
            isWinShown = true;
        }
    }

    void updateWinEnd() {
        if(!isWinShown) return;

        winTicks++;
        if(winTicks >= 180)
        {
            removeBlitzEntity(winEntityId);
            setCurrentStoryDefinitionFile("game/OUTRO.def", 0);
            setNewScreen(getStoryScreen());
        }
    }

    // Lose
    int loseEntityId;
    bool isLoseShown = false;
    int loseTicks = 0;

    void loadLose() {
        loseEntityId = addBlitzEntity(Vector3D(0, 0, 100));
        addBlitzMugenAnimationComponent(loseEntityId, &mSprites, &mAnimations, 61);
        setBlitzMugenAnimationVisibility(loseEntityId, 0);
    }

    void updateLose() {
        if(isWinShown) return;
        updateLoseStart();
        updateLoseEnd();
    }

    void updateLoseStart() {
        if(isLoseShown) return;

        auto carPosition = getBlitzEntityPosition(carEntityId);
        if(!checkPointInRectangle(GeoRectangle2D(-320, -240, 660, 500), carPosition.xy()))
        {
            setBlitzMugenAnimationVisibility(loseEntityId, 1);
            stopStreamingMusicFile();
            tryPlayMugenSound(&mSounds, 2, 0);
            isLoseShown = true;
        }
    }

    void updateLoseEnd() {
        if(!isLoseShown) return;

        loseTicks++;
        if(loseTicks >= 180)
        {
            removeBlitzEntity(loseEntityId);
            setNewScreen(getGameScreen());
        }
    }
};

EXPORT_SCREEN_CLASS(GameScreen);

void resetGame()
{
    gGameScreenData.mLevel = 0;
}