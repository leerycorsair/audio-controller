
#include <alsa/asoundlib.h>

struct audio_controller
{
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card;
    const char *selem_name;
} a_controller;

void audio_level_change(long audio_level)
{
    a_controller.card = "default";
    a_controller.selem_name = "Master";

    if (snd_mixer_open(&(a_controller.handle), 0) < 0)
        return;

    if (snd_mixer_attach(a_controller.handle, a_controller.card) < 0)
        return;

    if (snd_mixer_selem_register(a_controller.handle, NULL, NULL) < 0)
        return;

    if (snd_mixer_load(a_controller.handle) < 0)
        return;

    snd_mixer_selem_id_alloca(&(a_controller.sid));
    snd_mixer_selem_id_set_index(a_controller.sid, 0);
    snd_mixer_selem_id_set_name(a_controller.sid, a_controller.selem_name);

    snd_mixer_elem_t *elem = snd_mixer_find_selem(a_controller.handle, a_controller.sid);
    if (elem == NULL)
        return;

    snd_mixer_selem_get_playback_volume_range(elem, &(a_controller.min), &(a_controller.max));
    if (snd_mixer_selem_set_playback_volume_all(elem, audio_level * a_controller.max / 100) < 0)
        return;

    snd_mixer_close(a_controller.handle);
}

int main(int argc, char *argv[])
{
    audio_level_change(atol(argv[1]));
    return 0;
}