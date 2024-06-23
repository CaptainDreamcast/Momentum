#include "titlescreen.h"

#include <prism/blitz.h>
#include "gamescreen.h"
#include "storyscreen.h"

class TitleScreen
{
public:
    MugenSpriteFile mSprites;
    MugenAnimations mAnimations;
    MugenSounds mSounds;

    MugenAnimationHandlerElement* titleAnimation;

    TitleScreen() {
        mSprites = loadMugenSpriteFileWithoutPalette("game/TITLE.sff");
        mAnimations = loadMugenAnimationFile("game/TITLE.air");
        //mSounds = loadMugenSoundFile("game/TITLE.snd");

        titleAnimation = addMugenAnimation(getMugenAnimation(&mAnimations, 1), &mSprites, Vector3D(0, 0, 1));

        setWrapperTitleScreen(getTitleScreen());
        streamMusicFile("game/GAME.ogg");
    }
    void update() {
        updateInput();

    }

    void updateInput()
    {
            if(hasPressedAFlank() || hasPressedStartFlank())
            {
                resetGame();
                setCurrentStoryDefinitionFile("game/INTRO.def", 0);
                setNewScreen(getStoryScreen());
            }
    }
};

EXPORT_SCREEN_CLASS(TitleScreen);