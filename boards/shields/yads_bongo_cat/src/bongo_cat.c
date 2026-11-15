/*
 * Bongo cat for the YADS dongle screen, driven by key presses: a key on the left
 * half slams the left paw, a key on the right half the right paw. Paws go back up
 * when keys are released and the cat rests after a short pause.
 * Frames from https://github.com/hisa222/zmk-dongle-screen (MIT).
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <lvgl.h>

#include <zmk/display.h>
#include <zmk/display/status_screen.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

LV_IMG_DECLARE(bongo_cat_none);
LV_IMG_DECLARE(bongo_cat_left2);
LV_IMG_DECLARE(bongo_cat_right2);
LV_IMG_DECLARE(bongo_cat_both1);
LV_IMG_DECLARE(bongo_cat_both1_open);
LV_IMG_DECLARE(bongo_cat_both2);

static const void *idle_imgs[] = {&bongo_cat_both1_open, &bongo_cat_both1_open,
                                  &bongo_cat_both1_open, &bongo_cat_both1};

enum side { SIDE_LEFT, SIDE_RIGHT, SIDE_COUNT };

/* Key positions follow config/link.keymap: 12 per row, then 14 and 10 on the bottom rows. */
static enum side side_for_position(uint32_t position) {
    if (position < 36) {
        return position % 12 < 6 ? SIDE_LEFT : SIDE_RIGHT;
    }
    if (position < 50) {
        return position < 43 ? SIDE_LEFT : SIDE_RIGHT;
    }
    return position < 55 ? SIDE_LEFT : SIDE_RIGHT;
}

static struct k_spinlock lock;
static int held[SIDE_COUNT];
static int64_t last_press[SIDE_COUNT] = {INT64_MIN / 2, INT64_MIN / 2};
static int64_t last_activity = INT64_MIN / 2;

static lv_obj_t *bongo_cat;
static const void *shown_frame;

static void show_frame(const void *frame) {
    if (frame == shown_frame) {
        return;
    }
    shown_frame = frame;
    lv_anim_del(bongo_cat, NULL);
    if (frame == idle_imgs) {
        lv_animimg_set_src(bongo_cat, idle_imgs, ARRAY_SIZE(idle_imgs));
        lv_animimg_set_duration(bongo_cat, 10000);
        lv_animimg_set_repeat_count(bongo_cat, LV_ANIM_REPEAT_INFINITE);
        lv_animimg_start(bongo_cat);
    } else {
        lv_img_set_src(bongo_cat, frame);
    }
}

static void render(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(render_work, render);

static void render(struct k_work *work) {
    if (!bongo_cat) {
        return;
    }

    int64_t now = k_uptime_get();
    int64_t next = INT64_MAX;
    bool down[SIDE_COUNT];
    int64_t activity;

    K_SPINLOCK(&lock) {
        for (int i = 0; i < SIDE_COUNT; i++) {
            int64_t tap_end = last_press[i] + CONFIG_YADS_BONGO_CAT_MIN_TAP_MS;

            down[i] = held[i] > 0 || now < tap_end;
            if (held[i] == 0 && now < tap_end) {
                next = MIN(next, tap_end);
            }
        }
        activity = last_activity;
    }

    if (down[SIDE_LEFT] || down[SIDE_RIGHT]) {
        show_frame(down[SIDE_LEFT] && down[SIDE_RIGHT] ? &bongo_cat_both2
                   : down[SIDE_LEFT]                   ? &bongo_cat_left2
                                                       : &bongo_cat_right2);
    } else if (now < activity + CONFIG_YADS_BONGO_CAT_IDLE_MS) {
        show_frame(&bongo_cat_none);
        next = MIN(next, activity + CONFIG_YADS_BONGO_CAT_IDLE_MS);
    } else {
        show_frame(idle_imgs);
    }

    if (next != INT64_MAX) {
        k_work_reschedule_for_queue(zmk_display_work_q(), &render_work, K_MSEC(next - now));
    }
}

static int bongo_cat_position_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    enum side side = side_for_position(ev->position);
    int64_t now = k_uptime_get();

    K_SPINLOCK(&lock) {
        if (ev->state) {
            held[side]++;
            last_press[side] = now;
        } else if (held[side] > 0) {
            held[side]--;
        }
        last_activity = now;
    }

    if (zmk_display_is_initialized()) {
        k_work_reschedule_for_queue(zmk_display_work_q(), &render_work, K_NO_WAIT);
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(bongo_cat, bongo_cat_position_listener);
ZMK_SUBSCRIPTION(bongo_cat, zmk_position_state_changed);

lv_obj_t *yads_display_status_screen(void);

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = yads_display_status_screen();

    bongo_cat = lv_animimg_create(screen);
    lv_obj_align(bongo_cat, LV_ALIGN_TOP_MID, 0, 10);
    show_frame(idle_imgs);

    return screen;
}
