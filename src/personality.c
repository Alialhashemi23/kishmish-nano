#include "personality.h"
#include <stddef.h>

static uint32_t rng;
static CatPose pose;
static double pose_time, pose_left, idle_wait, line_wait, secret_wait, secret_cooldown;
static int secret_step, last_line, last_pool, last_neko;
static unsigned random_n(unsigned n) {
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng % n;
}
void personality_init(uint32_t seed) {
    rng = seed ? seed : 0x4b495348u;
    pose = POSE_IDLE;
    pose_time = pose_left = secret_wait = secret_cooldown = 0;
    secret_step = 0;
    last_line = last_pool = -1;
    last_neko = 0;
    idle_wait = 12 + random_n(13);
    line_wait = 25 + random_n(16);
}
void personality_pose_start(CatPose next) {
    pose = next;
    pose_time = 0;
    pose_left = next == POSE_BLINK ? .35 : next == POSE_LOAF ? 3.0 : 1.8;
    idle_wait = 12 + random_n(13);
}
CatPose personality_pose(void) {
    return pose;
}
double personality_pose_time(void) {
    return pose_time;
}
void personality_reset_secret(void) {
    secret_step = 0;
    secret_wait = 0;
}
int personality_secret(int key) {
    static const int sequence[] = {-1, 1, -1, 1, 3, 2};
    if (key == sequence[secret_step]) {
        secret_step++;
        secret_wait = 4;
    } else {
        secret_step = key == sequence[0] ? 1 : 0;
        secret_wait = secret_step ? 4 : 0;
    }
    if (secret_step != 6)
        return 0;
    personality_reset_secret();
    if (secret_cooldown > 0)
        return 2; /* Consume a repeated code without selecting Feed. */
    secret_cooldown = 20;
    line_wait = 25;
    personality_pose_start(POSE_SECRET);
    return 1;
}
static const char *pick(int pool, const char *const *lines, unsigned count) {
    unsigned index = random_n(count);
    if (last_pool == pool && (int)index == last_line)
        index = (index + 1) % count;
    last_pool = pool;
    last_line = (int)index;
    return lines[index];
}
const char *personality_tick(double dt, const Pet *pet, int blocked) {
    if (secret_cooldown > 0)
        secret_cooldown -= dt;
    if (secret_wait > 0) {
        secret_wait -= dt;
        if (secret_wait <= 0)
            personality_reset_secret();
    }
    if (pose_left > 0) {
        pose_time += dt;
        pose_left -= dt;
        if (pose_left <= 0)
            pose = POSE_IDLE;
    }
    if (blocked || pet->sleeping) {
        personality_reset_secret();
        if (pose != POSE_IDLE) {
            pose = POSE_IDLE;
            pose_left = 0;
        }
        return NULL;
    }
    idle_wait -= dt;
    line_wait -= dt;
    if (idle_wait <= 0 && pose == POSE_IDLE) {
        /* Most events are ordinary cat behavior. Rare faces stay rare. */
        if (random_n(12) == 0)
            personality_pose_start(pet->chaos > 35 ? POSE_GREMLIN : POSE_CURIOUS);
        else
            personality_pose_start((CatPose)(POSE_BLINK + random_n(5)));
    }
    if (line_wait > 0)
        return NULL;
    line_wait = 28 + random_n(18);
    static const char *const calm[] = {"A VERY GOOD DAY TO BE A CAT.", "I FIT. THEREFORE I SIT.",
                                       "THE SUNBEAM IS MINE.", "YOU MAY ADMIRE MY PAWS."};
    static const char *const needy[] = {"MY BOWL HAS A SUSPICIOUS GAP.",
                                        "ONE LITTLE SNACK. PLEASE.", "I REQUIRE YOUR ATTENTION.",
                                        "I HAVE BEEN SO BRAVE."};
    static const char *const tired[] = {"MY PAWS HAVE CLOCKED OUT.", "I COULD USE A LITTLE NAP.",
                                        "BEING ADORABLE IS HARD WORK."};
    static const char *const weird[] = {"THE MOON OWES ME TUNA.", "I KNOW WHAT THE PLANT DID.",
                                        "THE VOID HAS EXCELLENT TASTE.",
                                        "I HAVE COUNTED YOUR BONES."};
    static const char *const neko[] = {"HEADPATS. RIGHT MEOW.", "SNACK TIME NYA?",
                                       "PURRMISSION TO LOAF.", "LOOK AT MY LITTLE PEETS NYA."};
    int use_neko = !last_neko && random_n(4) == 0;
    last_neko = use_neko;
    if (use_neko) {
        if (pet->chaos > 35)
            return "THE MOON OWES ME TUNA NYA.";
        return pick(4, neko, 4);
    }
    if (pet->food < 30 || pet->joy < 30)
        return pick(1, needy, 4);
    if (pet->energy < 30)
        return pick(2, tired, 3);
    if (pet->chaos > 35)
        return pick(3, weird, 4);
    return pick(0, calm, 4);
}
const char *personality_react(CatReaction reaction, const Pet *pet) {
    line_wait = 25 + random_n(16); /* Ambient thoughts never overwrite a new reaction. */
    int neko = !last_neko && random_n(4) == 0;
    last_neko = neko;
    switch (reaction) {
    case REACT_FEED:
        personality_pose_start(POSE_MUNCH);
        return neko              ? "NOM NOM NYA!"
               : pet->chaos > 30 ? "ANOTHER OFFERING. GOOD."
                                 : "A SNACK! FOR ME?!";
    case REACT_PET:
        personality_pose_start(POSE_HAPPY);
        return neko                   ? "HEADPATS. RIGHT MEOW."
               : pet->affection >= 20 ? "MY FAVORITE HUMAN. ALWAYS."
               : pet->chaos > 30      ? "YOUR SECRETS ARE SAFE."
                                      : "YOU ARE MY FAVORITE.";
    case REACT_WIN:
        personality_pose_start(POSE_HAPPY);
        return neko              ? "A STAR! FOR MEW!"
               : pet->chaos > 30 ? "I SPARED THE MOON."
                                 : "YOU FOUND MY STAR!";
    case REACT_MISS:
        personality_pose_start(POSE_CURIOUS);
        return neko ? "NICE TRY NYA." : "A VERY GOOD WRONG STAR.";
    case REACT_WAKE:
        personality_pose_start(POSE_BLINK);
        return neko ? "GOOD MEOWNING." : "I DREAMED OF SNACKS.";
    }
    return "HELLO. I AM KISHMISH.";
}
const char *personality_greeting(const Pet *pet, long long away) {
    line_wait = 30;
    if (pet->sleeping)
        return "ZZZ. I SAVED YOU A DREAM.";
    if (pet->affection >= 20) {
        personality_pose_start(POSE_HAPPY);
        return away >= 600 ? "I MISSED MY FAVORITE HUMAN." : "OH GOOD. MY HUMAN IS HERE.";
    }
    if (pet->chaos > 35)
        return "YOU RETURNED. AS FORETOLD.";
    if (pet->visits >= 3)
        return "OUR LITTLE ROUTINE. I LIKE IT.";
    return "YOU CAME BACK. GOOD.";
}
