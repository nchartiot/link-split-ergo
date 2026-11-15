/*
 * Bongo cat for the YADS dongle screen, animated by typing speed.
 * Based on the widget from https://github.com/hisa222/zmk-dongle-screen (MIT).
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/display/status_screen.h>
#include <zmk/event_manager.h>
#include <zmk/events/wpm_state_changed.h>

LV_IMG_DECLARE(bongo_cat_none);
LV_IMG_DECLARE(bongo_cat_left1);
LV_IMG_DECLARE(bongo_cat_left2);
LV_IMG_DECLARE(bongo_cat_right1);
LV_IMG_DECLARE(bongo_cat_right2);
LV_IMG_DECLARE(bongo_cat_both1);
LV_IMG_DECLARE(bongo_cat_both1_open);
LV_IMG_DECLARE(bongo_cat_both2);

static const void *idle_imgs[] = {&bongo_cat_both1_open, &bongo_cat_both1_open,
                                  &bongo_cat_both1_open, &bongo_cat_both1};
static const void *slow_imgs[] = {&bongo_cat_left1,  &bongo_cat_both1, &bongo_cat_both1,
                                  &bongo_cat_right1, &bongo_cat_both1, &bongo_cat_both1,
                                  &bongo_cat_left1,  &bongo_cat_both1, &bongo_cat_both1};
static const void *mid_imgs[] = {&bongo_cat_left2,  &bongo_cat_left1,  &bongo_cat_none,
                                 &bongo_cat_right2, &bongo_cat_right1, &bongo_cat_none};
static const void *fast_imgs[] = {&bongo_cat_both2, &bongo_cat_both1, &bongo_cat_none,
                                  &bongo_cat_none};

enum bongo_anim { BONGO_ANIM_NONE, BONGO_ANIM_IDLE, BONGO_ANIM_SLOW, BONGO_ANIM_MID, BONGO_ANIM_FAST };

static const struct {
    const void **imgs;
    uint8_t len;
    uint32_t duration_ms;
} anims[] = {
    [BONGO_ANIM_IDLE] = {idle_imgs, ARRAY_SIZE(idle_imgs), 10000},
    [BONGO_ANIM_SLOW] = {slow_imgs, ARRAY_SIZE(slow_imgs), 2000},
    [BONGO_ANIM_MID] = {mid_imgs, ARRAY_SIZE(mid_imgs), 500},
    [BONGO_ANIM_FAST] = {fast_imgs, ARRAY_SIZE(fast_imgs), 200},
};

static lv_obj_t *bongo_cat;
static enum bongo_anim current_anim = BONGO_ANIM_NONE;

struct bongo_cat_state {
    int wpm;
};

static enum bongo_anim anim_for_wpm(int wpm) {
    if (wpm < 5) {
        return BONGO_ANIM_IDLE;
    }
    if (wpm < 30) {
        return BONGO_ANIM_SLOW;
    }
    return wpm < 70 ? BONGO_ANIM_MID : BONGO_ANIM_FAST;
}

static void bongo_cat_update_cb(struct bongo_cat_state state) {
    enum bongo_anim anim = anim_for_wpm(state.wpm);

    if (!bongo_cat || anim == current_anim) {
        return;
    }
    current_anim = anim;
    lv_animimg_set_src(bongo_cat, anims[anim].imgs, anims[anim].len);
    lv_animimg_set_duration(bongo_cat, anims[anim].duration_ms);
    lv_animimg_set_repeat_count(bongo_cat, LV_ANIM_REPEAT_INFINITE);
    lv_animimg_start(bongo_cat);
}

static struct bongo_cat_state bongo_cat_get_state(const zmk_event_t *eh) {
    const struct zmk_wpm_state_changed *ev = eh ? as_zmk_wpm_state_changed(eh) : NULL;

    return (struct bongo_cat_state){.wpm = ev ? ev->state : 0};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_bongo_cat, struct bongo_cat_state, bongo_cat_update_cb,
                            bongo_cat_get_state)
ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_wpm_state_changed);

lv_obj_t *yads_display_status_screen(void);

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = yads_display_status_screen();

    bongo_cat = lv_animimg_create(screen);
    lv_obj_align(bongo_cat, LV_ALIGN_TOP_MID, 0, 10);
    widget_bongo_cat_init();

    return screen;
}
