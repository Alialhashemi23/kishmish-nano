#ifndef PERSONALITY_H
#define PERSONALITY_H
#include "game.h"
typedef enum {
    POSE_IDLE,
    POSE_BLINK,
    POSE_EAR,
    POSE_TAIL,
    POSE_GROOM,
    POSE_LOAF,
    POSE_CURIOUS,
    POSE_GREMLIN,
    POSE_HAPPY,
    POSE_MUNCH,
    POSE_SECRET
} CatPose;
typedef enum { REACT_FEED, REACT_PET, REACT_WIN, REACT_MISS, REACT_WAKE } CatReaction;
void personality_init(uint32_t seed);
const char *personality_tick(double dt, const Pet *pet, int blocked);
const char *personality_react(CatReaction reaction, const Pet *pet);
const char *personality_greeting(const Pet *pet, long long away);
/* 1: triggered; 2: consumed during cooldown; 0: ordinary input. */
int personality_secret(int key);
void personality_reset_secret(void);
CatPose personality_pose(void);
double personality_pose_time(void);
/* Also used by the renderer preview and deterministic tests. */
void personality_pose_start(CatPose pose);
#endif
