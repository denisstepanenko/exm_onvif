#ifndef PLAYBACK_POOL_H
#define PLAYBACK_POOL_H


int playback_openfile(struct hd_playback_struct *phdplayback);
int playback_connect_media(struct hd_playback_struct *phdplayback);
int playbackreadpoolframe(struct hd_playback_struct *phdplayback, void *buf, int buf_len, int *seq, int *flag);
int playbackclosepool(struct hd_playback_struct *phdplayback);
#endif



