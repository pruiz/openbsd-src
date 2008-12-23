/*	$OpenBSD: azalia_codec.c,v 1.87 2008/12/23 10:01:14 jakemsr Exp $	*/
/*	$NetBSD: azalia_codec.c,v 1.8 2006/05/10 11:17:27 kent Exp $	*/

/*-
 * Copyright (c) 2005 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by TAMURA Kent
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/systm.h>
#include <uvm/uvm_param.h>
#include <dev/pci/azalia.h>

#define XNAME(co)	(((struct device *)co->az)->dv_xname)
#define MIXER_DELTA(n)	(AUDIO_MAX_GAIN / (n))

#define ENUM_OFFON	.un.e={2, {{{AudioNoff}, 0}, {{AudioNon}, 1}}}
#define ENUM_IO		.un.e={2, {{{"input"}, 0}, {{"output"}, 1}}}
#define AzaliaNfront	"front"
#define AzaliaNclfe	"clfe"
#define AzaliaNside	"side"

#define REALTEK_ALC660		0x10ec0660
#define ALC660_ASUS_G2K		0x13391043
#define REALTEK_ALC880		0x10ec0880
#define ALC880_ASUS_M5200	0x19931043
#define ALC880_ASUS_A7M		0x13231043
#define ALC880_MEDION_MD95257	0x203d161f
#define REALTEK_ALC882		0x10ec0882
#define ALC882_ASUS_A7T		0x13c21043
#define ALC882_ASUS_W2J		0x19711043
#define REALTEK_ALC883		0x10ec0883
#define ALC883_ACER_ID		0x00981025
#define REALTEK_ALC885		0x10ec0885
#define ALC885_APPLE_MB3	0x00a1106b
#define ALC885_APPLE_MB4	0x00a3106b
#define SIGMATEL_STAC9221	0x83847680
#define STAC9221_APPLE_ID	0x76808384
#define SIGMATEL_STAC9205	0x838476a0
#define STAC9205_DELL_D630	0x01f91028
#define STAC9205_DELL_V1500	0x02281028

int	azalia_generic_codec_init_dacgroup(codec_t *);
int	azalia_generic_codec_fnode(codec_t *, nid_t, int, int);
int	azalia_generic_codec_add_convgroup(codec_t *, convgroupset_t *,
    uint32_t, uint32_t);
int	azalia_generic_mixer_init(codec_t *);
int	azalia_generic_mixer_autoinit(codec_t *);

int	azalia_generic_mixer_fix_indexes(codec_t *);
int	azalia_generic_mixer_default(codec_t *);
int	azalia_generic_mixer_pin_sense(codec_t *);
int	azalia_generic_mixer_create_virtual(codec_t *, int, int);
int	azalia_generic_mixer_delete(codec_t *);
int	azalia_generic_mixer_ensure_capacity(codec_t *, size_t);
int	azalia_generic_mixer_get(const codec_t *, nid_t, int, mixer_ctrl_t *);
int	azalia_generic_mixer_set(codec_t *, nid_t, int, const mixer_ctrl_t *);
u_char	azalia_generic_mixer_from_device_value
	(const codec_t *, nid_t, int, uint32_t );
uint32_t azalia_generic_mixer_to_device_value
	(const codec_t *, nid_t, int, u_char);
int	azalia_generic_set_port(codec_t *, mixer_ctrl_t *);
int	azalia_generic_get_port(codec_t *, mixer_ctrl_t *);
int	azalia_gpio_unmute(codec_t *, int);

void	azalia_pin_config_ov(widget_t *, int, int);

int	azalia_alc260_mixer_init(codec_t *);
int	azalia_alc88x_mixer_init(codec_t *);

int
azalia_codec_init_vtbl(codec_t *this)
{
	/**
	 * We can refer this->vid and this->subid.
	 */
	this->name = NULL;
	this->init_dacgroup = azalia_generic_codec_init_dacgroup;
	this->mixer_init = azalia_generic_mixer_autoinit;
	this->mixer_delete = azalia_generic_mixer_delete;
	this->set_port = azalia_generic_set_port;
	this->get_port = azalia_generic_get_port;
	this->unsol_event = NULL;
	switch (this->vid) {
	case 0x10ec0260:
		this->name = "Realtek ALC260";
		this->mixer_init = azalia_alc260_mixer_init;
		break;
	case 0x10ec0268:
		this->name = "Realtek ALC268";
		break;
	case 0x10ec0269:
		this->name = "Realtek ALC269";
		break;
	case 0x10ec0662:
		this->name = "Realtek ALC662-GR";
		break;
	case 0x10ec0861:
		this->name = "Realtek ALC861";
		break;
	case 0x10ec0880:
		this->name = "Realtek ALC880";
		this->mixer_init = azalia_alc88x_mixer_init;
		break;
	case 0x10ec0882:
		this->name = "Realtek ALC882";
		this->mixer_init = azalia_alc88x_mixer_init;
		break;
	case 0x10ec0883:
		this->name = "Realtek ALC883";
		this->mixer_init = azalia_alc88x_mixer_init;
		break;
	case 0x10ec0885:
		this->name = "Realtek ALC885";
		this->mixer_init = azalia_alc88x_mixer_init;
		break;
	case 0x10ec0888:
		this->name = "Realtek ALC888";
		this->mixer_init = azalia_alc88x_mixer_init;
		break;
	case 0x111d76b2:
		this->name = "IDT 92HD71B7";
		break;
	case 0x111d76b6:
		this->name = "IDT 92HD71B5";
		break;
	case 0x11d41884:
		this->name = "Analog Devices AD1884";
		break;
	case 0x11d4194a:
		this->name = "Analog Devices AD1984A";
		break;
	case 0x11d41981:
		this->name = "Analog Devices AD1981HD";
		break;
	case 0x11d41983:
		this->name = "Analog Devices AD1983";
		break;
	case 0x11d41984:
		this->name = "Analog Devices AD1984";
		break;
	case 0x11d41988:
		this->name = "Analog Devices AD1988A";
		break;
	case 0x11d4198b:
		this->name = "Analog Devices AD1988B";
		break;
	case 0x14f15045:
		this->name = "Conexant CX20549";  /* Venice */
		break;
	case 0x14f15047:
		this->name = "Conexant CX20551";  /* Waikiki */
		break;
	case 0x14f15051:
		this->name = "Conexant CX20561";  /* Hermosa */
		break;
	case 0x434d4980:
		this->name = "CMedia CMI9880";
		break;
	case 0x83847616:
		this->name = "Sigmatel STAC9228X";
		break;
	case 0x83847617:
		this->name = "Sigmatel STAC9228D";
		break;
	case 0x83847618:
		this->name = "Sigmatel STAC9227X";
		break;
	case 0x83847620:
		this->name = "Sigmatel STAC9274";
		break;
	case 0x83847621:
		this->name = "Sigmatel STAC9274D";
		break;
	case 0x83847626:
		this->name = "Sigmatel STAC9271X";
		break;
	case 0x83847627:
		this->name = "Sigmatel STAC9271D";
		break;
	case 0x83847632:
		this->name = "Sigmatel STAC9202";
		/* master volume at nid 0e */
		/* record volume at nid 09 */
		break;
	case 0x83847634:
		this->name = "Sigmatel STAC9250";
		/* master volume at nid 0e */
		/* record volume at nid 09 */
		break;
	case 0x83847636:
		this->name = "Sigmatel STAC9251";
		/* master volume at nid 0e */
		/* record volume at nid 09 */
		break;
	case 0x83847661:
		/* FALLTHRU */
	case 0x83847662:
		this->name = "Sigmatel STAC9225";
		break;
	case 0x83847680:
		this->name = "Sigmatel STAC9221";
		break;
	case 0x83847683:
		this->name = "Sigmatel STAC9221D";
		break;
	case 0x83847690:
		this->name = "Sigmatel STAC9200";
		break;
	case 0x83847691:
		this->name = "Sigmatel STAC9200D";
		break;
	case 0x838476a0:
		this->name = "Sigmatel STAC9205X";
		break;
	case 0x838476a1:
		this->name = "Sigmatel STAC9205D";
		break;
	case 0x838476a2:
		this->name = "Sigmatel STAC9204X";
		break;
	case 0x838476a3:
		this->name = "Sigmatel STAC9204D";
		break;
	}
	return 0;
}

/* ----------------------------------------------------------------
 * functions for generic codecs
 * ---------------------------------------------------------------- */

int
azalia_widget_enabled(const codec_t *this, nid_t nid)
{
	if (!VALID_WIDGET_NID(nid, this) || !this->w[nid].enable)
		return 0;
	return 1;
}


int
azalia_generic_codec_init_dacgroup(codec_t *this)
{
	this->dacs.ngroups = 0;
	azalia_generic_codec_add_convgroup(this, &this->dacs,
	    COP_AWTYPE_AUDIO_OUTPUT, 0);
	azalia_generic_codec_add_convgroup(this, &this->dacs,
	    COP_AWTYPE_AUDIO_OUTPUT, COP_AWCAP_DIGITAL);
	this->dacs.cur = 0;

	this->adcs.ngroups = 0;
	azalia_generic_codec_add_convgroup(this, &this->adcs,
	    COP_AWTYPE_AUDIO_INPUT, 0);
	azalia_generic_codec_add_convgroup(this, &this->adcs,
	    COP_AWTYPE_AUDIO_INPUT, COP_AWCAP_DIGITAL);
	this->adcs.cur = 0;

	return 0;
}

int
azalia_generic_codec_add_convgroup(codec_t *this, convgroupset_t *group,
    uint32_t type, uint32_t digital)
{
	nid_t all_convs[HDA_MAX_CHANNELS];
	int nall_convs;
	nid_t convs[HDA_MAX_CHANNELS];
	int nconvs;
	int assoc, seq;
	nid_t conv;
	int i, j, k;

	DPRINTF(("%s: looking for %s %s\n", __func__,
	    digital ? "digital" : "analog",
	    (type == COP_AWTYPE_AUDIO_OUTPUT) ? "DACs" : "ADCs"));

	nconvs = 0;
	nall_convs = 0;

	FOR_EACH_WIDGET(this, i) {
		if (this->w[i].type == type &&
		    (this->w[i].widgetcap & COP_AWCAP_DIGITAL) == digital &&
		    nall_convs < HDA_MAX_CHANNELS)
			all_convs[nall_convs++] = this->w[i].nid;
	}
	if (nall_convs == 0)
		goto done;

	for (assoc = 0; assoc <= CORB_CD_ASSOCIATION_MAX; assoc++) {
		for (seq = 0; seq <= CORB_CD_SEQUENCE_MAX; seq++) {
			FOR_EACH_WIDGET(this, i) {
				const widget_t *w;

				w = &this->w[i];
				if (!w->enable)
					continue;
				if (w->type != COP_AWTYPE_PIN_COMPLEX)
					continue;
				if ((w->widgetcap &
				    COP_AWCAP_DIGITAL) != digital)
					continue;
				if (w->d.pin.sequence != seq ||
				    w->d.pin.association != assoc)
					continue;
				if (type == COP_AWTYPE_AUDIO_OUTPUT) {
					if (!(w->d.pin.cap & COP_PINCAP_OUTPUT))
						continue;
				} else {
					if (!(w->d.pin.cap & COP_PINCAP_INPUT))
						continue;
				}
				DPRINTF(("\tpin=%2.2x, assoc=%d, seq=%d:",
				    w->nid, assoc, seq));
				for (j = 0; j < nall_convs; j++) {
					conv = all_convs[j];
					for (k = 0; k < nconvs; k++) {
						if (convs[k] == conv)
							break;
					}
					if (k < nconvs)
						continue;
					if (type == COP_AWTYPE_AUDIO_OUTPUT) {
						k = azalia_generic_codec_fnode
						    (this, conv, i, 0);
						if (k < 0)
							continue;
					} else {
						if (!azalia_widget_enabled(this,
						    conv))
							continue;
						k = azalia_generic_codec_fnode
						    (this, w->nid, conv, 0);
						if (k < 0)
							continue;
					}
					convs[nconvs++] = conv;
					DPRINTF(("%2.2x", conv));
					if (nconvs >= nall_convs ||
					    nconvs >= HDA_MAX_CHANNELS) {
						DPRINTF(("\n"));
						goto done;
					}
				}
				DPRINTF(("\n"));
			}
		}
	}
done:
	for (i = 0; i < nconvs; i++)
		group->groups[group->ngroups].conv[i] = convs[i];
	if (nconvs > 0) {
		group->groups[group->ngroups].nconv = i;
		group->ngroups++;
	}

	for (i = 0; i < nall_convs; i++) {
		for (j = 0; j < nconvs; j++)
			if (convs[j] == all_convs[i])
				break;
		if (j == nconvs)
			this->w[all_convs[i]].enable = 0;
	}

	return 0;
}

int
azalia_generic_codec_fnode(codec_t *this, nid_t node, int index, int depth)
{
	const widget_t *w;
	int i, ret;

	w = &this->w[index];
	if (w->nid == node) {
		DPRINTF((" depth=%d:", depth));
		return index;
	}
	/* back at the beginning or a bad end */
	if (depth > 0 &&
	    (w->type == COP_AWTYPE_PIN_COMPLEX ||
	    w->type == COP_AWTYPE_AUDIO_OUTPUT ||
	    w->type == COP_AWTYPE_AUDIO_INPUT))
		return -1;
	if (++depth >= 10)
		return -1;
	for (i = 0; i < w->nconnections; i++) {
		if (!azalia_widget_enabled(this, w->connections[i]))
			continue;
		ret = azalia_generic_codec_fnode(this, node,
		    w->connections[i], depth);
		if (ret >= 0)
			return ret;
	}
	return -1;
}

/* ----------------------------------------------------------------
 * Generic mixer functions
 * ---------------------------------------------------------------- */

int
azalia_generic_mixer_init(codec_t *this)
{
	/*
	 * pin		"<color>%2.2x"
	 * audio output	"dac%2.2x"
	 * audio input	"adc%2.2x"
	 * mixer	"mixer%2.2x"
	 * selector	"sel%2.2x"
	 */
	mixer_item_t *m;
	int err, i, j, k;

	this->maxmixers = 10;
	this->nmixers = 0;
	this->mixers = malloc(sizeof(mixer_item_t) * this->maxmixers,
	    M_DEVBUF, M_NOWAIT | M_ZERO);
	if (this->mixers == NULL) {
		printf("%s: out of memory in %s\n", XNAME(this), __func__);
		return ENOMEM;
	}

	/* register classes */
	m = &this->mixers[AZ_CLASS_INPUT];
	m->devinfo.index = AZ_CLASS_INPUT;
	strlcpy(m->devinfo.label.name, AudioCinputs,
	    sizeof(m->devinfo.label.name));
	m->devinfo.type = AUDIO_MIXER_CLASS;
	m->devinfo.mixer_class = AZ_CLASS_INPUT;
	m->devinfo.next = AUDIO_MIXER_LAST;
	m->devinfo.prev = AUDIO_MIXER_LAST;
	m->nid = 0;

	m = &this->mixers[AZ_CLASS_OUTPUT];
	m->devinfo.index = AZ_CLASS_OUTPUT;
	strlcpy(m->devinfo.label.name, AudioCoutputs,
	    sizeof(m->devinfo.label.name));
	m->devinfo.type = AUDIO_MIXER_CLASS;
	m->devinfo.mixer_class = AZ_CLASS_OUTPUT;
	m->devinfo.next = AUDIO_MIXER_LAST;
	m->devinfo.prev = AUDIO_MIXER_LAST;
	m->nid = 0;

	m = &this->mixers[AZ_CLASS_RECORD];
	m->devinfo.index = AZ_CLASS_RECORD;
	strlcpy(m->devinfo.label.name, AudioCrecord,
	    sizeof(m->devinfo.label.name));
	m->devinfo.type = AUDIO_MIXER_CLASS;
	m->devinfo.mixer_class = AZ_CLASS_RECORD;
	m->devinfo.next = AUDIO_MIXER_LAST;
	m->devinfo.prev = AUDIO_MIXER_LAST;
	m->nid = 0;

	this->nmixers = AZ_CLASS_RECORD + 1;

#define MIXER_REG_PROLOG	\
	mixer_devinfo_t *d; \
	err = azalia_generic_mixer_ensure_capacity(this, this->nmixers + 1); \
	if (err) \
		return err; \
	m = &this->mixers[this->nmixers]; \
	d = &m->devinfo; \
	m->nid = i

	FOR_EACH_WIDGET(this, i) {
		const widget_t *w;

		w = &this->w[i];
		if (!w->enable)
			continue;

		/* selector */
		if (w->type != COP_AWTYPE_AUDIO_MIXER && w->nconnections > 0 &&
		    !(w->nconnections == 1 &&
		    azalia_widget_enabled(this, w->connections[0]) &&
		    strcmp(w->name, this->w[w->connections[0]].name) == 0)) {
			MIXER_REG_PROLOG;
			snprintf(d->label.name, sizeof(d->label.name),
			    "%s_source", w->name);
			d->type = AUDIO_MIXER_ENUM;
			if (w->mixer_class >= 0)
				d->mixer_class = w->mixer_class;
			else {
				if (w->type == COP_AWTYPE_AUDIO_SELECTOR)
					d->mixer_class = AZ_CLASS_INPUT;
				else
					d->mixer_class = AZ_CLASS_OUTPUT;
			}
			m->target = MI_TARGET_CONNLIST;
			for (j = 0, k = 0; j < w->nconnections && k < 32; j++) {
				if (!azalia_widget_enabled(this,
				    w->connections[j]))
					continue;
				d->un.e.member[k].ord = j;
				strlcpy(d->un.e.member[k].label.name,
				    this->w[w->connections[j]].name,
				    MAX_AUDIO_DEV_LEN);
				k++;
			}
			d->un.e.num_mem = k;
			this->nmixers++;
		}

		/* output mute */
		if (w->widgetcap & COP_AWCAP_OUTAMP &&
		    w->outamp_cap & COP_AMPCAP_MUTE) {
			MIXER_REG_PROLOG;
			snprintf(d->label.name, sizeof(d->label.name),
			    "%s_mute", w->name);
			d->type = AUDIO_MIXER_ENUM;
			if (w->mixer_class >= 0)
				d->mixer_class = w->mixer_class;
			else {
				if (w->type == COP_AWTYPE_AUDIO_MIXER ||
				    w->type == COP_AWTYPE_AUDIO_SELECTOR ||
				    w->type == COP_AWTYPE_PIN_COMPLEX)
					d->mixer_class = AZ_CLASS_OUTPUT;
				else
					d->mixer_class = AZ_CLASS_INPUT;
			}
			m->target = MI_TARGET_OUTAMP;
			d->un.e.num_mem = 2;
			d->un.e.member[0].ord = 0;
			strlcpy(d->un.e.member[0].label.name, AudioNoff,
			    MAX_AUDIO_DEV_LEN);
			d->un.e.member[1].ord = 1;
			strlcpy(d->un.e.member[1].label.name, AudioNon,
			    MAX_AUDIO_DEV_LEN);
			this->nmixers++;
		}

		/* output gain */
		if (w->widgetcap & COP_AWCAP_OUTAMP
		    && COP_AMPCAP_NUMSTEPS(w->outamp_cap)) {
			MIXER_REG_PROLOG;
			snprintf(d->label.name, sizeof(d->label.name),
			    "%s", w->name);
			d->type = AUDIO_MIXER_VALUE;
			if (w->mixer_class >= 0)
				d->mixer_class = w->mixer_class;
			else {
				if (w->type == COP_AWTYPE_AUDIO_MIXER ||
				    w->type == COP_AWTYPE_AUDIO_SELECTOR ||
				    w->type == COP_AWTYPE_PIN_COMPLEX)
					d->mixer_class = AZ_CLASS_OUTPUT;
				else
					d->mixer_class = AZ_CLASS_INPUT;
			}
			m->target = MI_TARGET_OUTAMP;
			d->un.v.num_channels = WIDGET_CHANNELS(w);
			d->un.v.units.name[0] = 0;
			d->un.v.delta =
			    MIXER_DELTA(COP_AMPCAP_NUMSTEPS(w->outamp_cap));
			this->nmixers++;
		}

		/* input mute */
		if (w->widgetcap & COP_AWCAP_INAMP &&
		    w->inamp_cap & COP_AMPCAP_MUTE) {
			if (w->type != COP_AWTYPE_AUDIO_SELECTOR &&
			    w->type != COP_AWTYPE_AUDIO_MIXER) {
				MIXER_REG_PROLOG;
				snprintf(d->label.name, sizeof(d->label.name),
				    "%s_mute", w->name);
				d->type = AUDIO_MIXER_ENUM;
				if (w->mixer_class >= 0)
					d->mixer_class = w->mixer_class;
				else
					d->mixer_class = AZ_CLASS_INPUT;
				m->target = 0;
				d->un.e.num_mem = 2;
				d->un.e.member[0].ord = 0;
				strlcpy(d->un.e.member[0].label.name,
				    AudioNoff, MAX_AUDIO_DEV_LEN);
				d->un.e.member[1].ord = 1;
				strlcpy(d->un.e.member[1].label.name,
				    AudioNon, MAX_AUDIO_DEV_LEN);
				this->nmixers++;
			} else {
				MIXER_REG_PROLOG;
				snprintf(d->label.name, sizeof(d->label.name),
				    "%s_source", w->name);
				m->target = MI_TARGET_MUTESET;
				d->type = AUDIO_MIXER_SET;
				if (w->mixer_class >= 0)
					d->mixer_class = w->mixer_class;
				else
					d->mixer_class = AZ_CLASS_INPUT;
				for (j = 0, k = 0;
				    j < w->nconnections && k < 32; j++) {
					if (!azalia_widget_enabled(this,
					    w->connections[j]))
						continue;
					d->un.s.member[k].mask = 1 << j;
					strlcpy(d->un.s.member[k].label.name,
					    this->w[w->connections[j]].name,
					    MAX_AUDIO_DEV_LEN);
					k++;
				}
				d->un.s.num_mem = k;
				if (k != 0)
					this->nmixers++;
			}
		}

		/* input gain */
		if (w->widgetcap & COP_AWCAP_INAMP
		    && COP_AMPCAP_NUMSTEPS(w->inamp_cap)) {
			if (w->type != COP_AWTYPE_AUDIO_SELECTOR &&
			    w->type != COP_AWTYPE_AUDIO_MIXER) {
				MIXER_REG_PROLOG;
				snprintf(d->label.name, sizeof(d->label.name),
				    "%s", w->name);
				d->type = AUDIO_MIXER_VALUE;
				if (w->mixer_class >= 0)
					d->mixer_class = w->mixer_class;
				else
					d->mixer_class = AZ_CLASS_INPUT;
				m->target = 0;
				d->un.v.num_channels = WIDGET_CHANNELS(w);
				d->un.v.units.name[0] = 0;
				d->un.v.delta =
				    MIXER_DELTA(COP_AMPCAP_NUMSTEPS(w->inamp_cap));
				this->nmixers++;
			} else {
				for (j = 0; j < w->nconnections; j++) {
					if (!azalia_widget_enabled(this,
					    w->connections[j]))
						continue;
					MIXER_REG_PROLOG;
					snprintf(d->label.name,
					    sizeof(d->label.name), "%s_%s",
					    w->name,
					    this->w[w->connections[j]].name);
					d->type = AUDIO_MIXER_VALUE;
					if (w->mixer_class >= 0)
						d->mixer_class = w->mixer_class;
					else
						d->mixer_class = AZ_CLASS_INPUT;
					m->target = j;
					d->un.v.num_channels = WIDGET_CHANNELS(w);
					d->un.v.units.name[0] = 0;
					d->un.v.delta =
					    MIXER_DELTA(COP_AMPCAP_NUMSTEPS(w->inamp_cap));
					this->nmixers++;
				}
			}
		}

		/* pin direction */
		if (w->type == COP_AWTYPE_PIN_COMPLEX &&
		    w->d.pin.cap & COP_PINCAP_OUTPUT &&
		    w->d.pin.cap & COP_PINCAP_INPUT) {
			MIXER_REG_PROLOG;
			snprintf(d->label.name, sizeof(d->label.name),
			    "%s_dir", w->name);
			d->type = AUDIO_MIXER_ENUM;
			d->mixer_class = AZ_CLASS_OUTPUT;
			m->target = MI_TARGET_PINDIR;
			d->un.e.num_mem = 2;
			d->un.e.member[0].ord = 0;
			strlcpy(d->un.e.member[0].label.name, AudioNinput,
			    MAX_AUDIO_DEV_LEN);
			d->un.e.member[1].ord = 1;
			strlcpy(d->un.e.member[1].label.name, AudioNoutput,
			    MAX_AUDIO_DEV_LEN);
			this->nmixers++;
		}

		/* pin headphone-boost */
		if (w->type == COP_AWTYPE_PIN_COMPLEX &&
		    w->d.pin.cap & COP_PINCAP_HEADPHONE) {
			MIXER_REG_PROLOG;
			snprintf(d->label.name, sizeof(d->label.name),
			    "%s_boost", w->name);
			d->type = AUDIO_MIXER_ENUM;
			d->mixer_class = AZ_CLASS_OUTPUT;
			m->target = MI_TARGET_PINBOOST;
			d->un.e.num_mem = 2;
			d->un.e.member[0].ord = 0;
			strlcpy(d->un.e.member[0].label.name, AudioNoff,
			    MAX_AUDIO_DEV_LEN);
			d->un.e.member[1].ord = 1;
			strlcpy(d->un.e.member[1].label.name, AudioNon,
			    MAX_AUDIO_DEV_LEN);
			this->nmixers++;
		}

		if (w->type == COP_AWTYPE_PIN_COMPLEX &&
		    w->d.pin.cap & COP_PINCAP_EAPD) {
			MIXER_REG_PROLOG;
			snprintf(d->label.name, sizeof(d->label.name),
			    "%s_eapd", w->name);
			d->type = AUDIO_MIXER_ENUM;
			d->mixer_class = AZ_CLASS_OUTPUT;
			m->target = MI_TARGET_EAPD;
			d->un.e.num_mem = 2;
			d->un.e.member[0].ord = 0;
			strlcpy(d->un.e.member[0].label.name, AudioNoff,
			    MAX_AUDIO_DEV_LEN);
			d->un.e.member[1].ord = 1;
			strlcpy(d->un.e.member[1].label.name, AudioNon,
			    MAX_AUDIO_DEV_LEN);
			this->nmixers++;
		}

		/* volume knob */
		if (w->type == COP_AWTYPE_VOLUME_KNOB &&
		    w->d.volume.cap & COP_VKCAP_DELTA) {
			MIXER_REG_PROLOG;
			strlcpy(d->label.name, w->name, sizeof(d->label.name));
			d->type = AUDIO_MIXER_VALUE;
			d->mixer_class = AZ_CLASS_OUTPUT;
			m->target = MI_TARGET_VOLUME;
			d->un.v.num_channels = 1;
			d->un.v.units.name[0] = 0;
			d->un.v.delta =
			    MIXER_DELTA(COP_VKCAP_NUMSTEPS(w->d.volume.cap));
			this->nmixers++;
		}
	}

	/* sense pins */
	for (i = 0; i < this->nsense_pins; i++) {
		if (!azalia_widget_enabled(this, this->sense_pins[i])) {
			DPRINTF(("%s: sense pin %2.2x not found\n",
			    __func__, this->sense_pins[i]));
			continue;
		}

		MIXER_REG_PROLOG;
		m->nid = this->w[this->sense_pins[i]].nid;
		snprintf(d->label.name, sizeof(d->label.name), "%s_sense",
		    this->w[this->sense_pins[i]].name);
		d->type = AUDIO_MIXER_ENUM;
		d->mixer_class = AZ_CLASS_OUTPUT;
		m->target = MI_TARGET_PINSENSE;
		d->un.e.num_mem = 2;
		d->un.e.member[0].ord = 0;
		strlcpy(d->un.e.member[0].label.name, "unplugged",
		    MAX_AUDIO_DEV_LEN);
		d->un.e.member[1].ord = 1;
		strlcpy(d->un.e.member[1].label.name, "plugged",
		    MAX_AUDIO_DEV_LEN);
		this->nmixers++;
	}

	/* if the codec has multiple DAC groups, create "inputs.usingdac" */
	if (this->dacs.ngroups > 1) {
		MIXER_REG_PROLOG;
		strlcpy(d->label.name, "usingdac", sizeof(d->label.name));
		d->type = AUDIO_MIXER_ENUM;
		d->mixer_class = AZ_CLASS_INPUT;
		m->target = MI_TARGET_DAC;
		for (i = 0; i < this->dacs.ngroups && i < 32; i++) {
			d->un.e.member[i].ord = i;
			for (j = 0; j < this->dacs.groups[i].nconv; j++) {
				if (j * 2 >= MAX_AUDIO_DEV_LEN)
					break;
				snprintf(d->un.e.member[i].label.name + j*2,
				    MAX_AUDIO_DEV_LEN - j*2, "%2.2x",
				    this->dacs.groups[i].conv[j]);
			}
		}
		d->un.e.num_mem = i;
		this->nmixers++;
	}

	/* if the codec has multiple ADC groups, create "record.usingadc" */
	if (this->adcs.ngroups > 1) {
		MIXER_REG_PROLOG;
		strlcpy(d->label.name, "usingadc", sizeof(d->label.name));
		d->type = AUDIO_MIXER_ENUM;
		d->mixer_class = AZ_CLASS_RECORD;
		m->target = MI_TARGET_ADC;
		for (i = 0; i < this->adcs.ngroups && i < 32; i++) {
			d->un.e.member[i].ord = i;
			for (j = 0; j < this->adcs.groups[i].nconv; j++) {
				if (j * 2 >= MAX_AUDIO_DEV_LEN)
					break;
				snprintf(d->un.e.member[i].label.name + j*2,
				    MAX_AUDIO_DEV_LEN - j*2, "%2.2x",
				    this->adcs.groups[i].conv[j]);
			}
		}
		d->un.e.num_mem = i;
		this->nmixers++;
	}

	azalia_generic_mixer_fix_indexes(this);
	azalia_generic_mixer_default(this);
	return 0;
}

int
azalia_generic_mixer_ensure_capacity(codec_t *this, size_t newsize)
{
	size_t newmax;
	void *newbuf;

	if (this->maxmixers >= newsize)
		return 0;
	newmax = this->maxmixers + 10;
	if (newmax < newsize)
		newmax = newsize;
	newbuf = malloc(sizeof(mixer_item_t) * newmax, M_DEVBUF, M_NOWAIT | M_ZERO);
	if (newbuf == NULL) {
		printf("%s: out of memory in %s\n", XNAME(this), __func__);
		return ENOMEM;
	}
	bcopy(this->mixers, newbuf, this->maxmixers * sizeof(mixer_item_t));
	free(this->mixers, M_DEVBUF);
	this->mixers = newbuf;
	this->maxmixers = newmax;
	return 0;
}

int
azalia_generic_mixer_fix_indexes(codec_t *this)
{
	int i;
	mixer_devinfo_t *d;

	for (i = 0; i < this->nmixers; i++) {
		d = &this->mixers[i].devinfo;
#ifdef DIAGNOSTIC
		if (d->index != 0 && d->index != i)
			printf("%s: index mismatch %d %d\n", __func__,
			    d->index, i);
#endif
		d->index = i;
		if (d->prev == 0)
			d->prev = AUDIO_MIXER_LAST;
		if (d->next == 0)
			d->next = AUDIO_MIXER_LAST;
	}
	return 0;
}

int
azalia_generic_mixer_default(codec_t *this)
{
	widget_t *w;
	mixer_item_t *m;
	mixer_ctrl_t mc;
	int i, j;

	/* unmute all */
	for (i = 0; i < this->nmixers; i++) {
		m = &this->mixers[i];
		if (!IS_MI_TARGET_INAMP(m->target) &&
		    m->target != MI_TARGET_OUTAMP)
			continue;
		if (m->devinfo.type != AUDIO_MIXER_ENUM)
			continue;
		bzero(&mc, sizeof(mc));
		mc.dev = i;
		mc.type = AUDIO_MIXER_ENUM;
		azalia_generic_mixer_set(this, m->nid, m->target, &mc);
	}

	/* set unextreme volume */
	for (i = 0; i < this->nmixers; i++) {
		m = &this->mixers[i];
		if (!IS_MI_TARGET_INAMP(m->target) &&
		    m->target != MI_TARGET_OUTAMP &&
		    m->target != MI_TARGET_VOLUME)
			continue;
		if (m->devinfo.type != AUDIO_MIXER_VALUE)
			continue;
		bzero(&mc, sizeof(mc));
		mc.dev = i;
		mc.type = AUDIO_MIXER_VALUE;
		mc.un.value.num_channels = 1;
		mc.un.value.level[0] = AUDIO_MAX_GAIN / 2;
		if (m->target != MI_TARGET_VOLUME &&
		    WIDGET_CHANNELS(&this->w[m->nid]) == 2) {
			mc.un.value.num_channels = 2;
			mc.un.value.level[1] = mc.un.value.level[0];
		}
		azalia_generic_mixer_set(this, m->nid, m->target, &mc);
	}

	/* unmute all */
	for (i = 0; i < this->nmixers; i++) {
		m = &this->mixers[i];
		if (m->target != MI_TARGET_MUTESET)
			continue;
		if (m->devinfo.type != AUDIO_MIXER_SET)
			continue;
		bzero(&mc, sizeof(mc));
		mc.dev = i;
		mc.type = AUDIO_MIXER_SET;
		if (!azalia_widget_enabled(this, m->nid)) {
			DPRINTF(("%s: invalid muteset nid\n", __func__));
			return EINVAL;
		}
		w = &this->w[m->nid];
		for (j = 0; j < w->nconnections; j++) {
			if (!azalia_widget_enabled(this, w->connections[j]))
				continue;
			mc.un.mask |= 1 << j;
		}
		azalia_generic_mixer_set(this, m->nid, m->target, &mc);
	}

	return 0;
}

int
azalia_generic_mixer_pin_sense(codec_t *this)
{
	/* GPIO quirks */

	if (this->vid == SIGMATEL_STAC9221 && this->subid == STAC9221_APPLE_ID) {
		this->comresp(this, this->audiofunc, 0x7e7, 0, NULL);
		azalia_gpio_unmute(this, 0);
		azalia_gpio_unmute(this, 1);
	}
	if (this->vid == REALTEK_ALC883 && this->subid == ALC883_ACER_ID) {
		azalia_gpio_unmute(this, 0);
		azalia_gpio_unmute(this, 1);
	}
	if ((this->vid == REALTEK_ALC660 && this->subid == ALC660_ASUS_G2K) ||
	    (this->vid == REALTEK_ALC880 && this->subid == ALC880_ASUS_M5200) ||
	    (this->vid == REALTEK_ALC880 && this->subid == ALC880_ASUS_A7M) ||
	    (this->vid == REALTEK_ALC882 && this->subid == ALC882_ASUS_A7T) ||
	    (this->vid == REALTEK_ALC882 && this->subid == ALC882_ASUS_W2J) ||
	    (this->vid == REALTEK_ALC885 && this->subid == ALC885_APPLE_MB3) ||
	    (this->vid == REALTEK_ALC885 && this->subid == ALC885_APPLE_MB4) ||
	    (this->vid == SIGMATEL_STAC9205 && this->subid == STAC9205_DELL_D630) ||
	    (this->vid == SIGMATEL_STAC9205 && this->subid == STAC9205_DELL_V1500)) {
		azalia_gpio_unmute(this, 0);
	}

	if (this->vid == REALTEK_ALC880 && this->subid == ALC880_MEDION_MD95257) {
		azalia_gpio_unmute(this, 1);
	}

	return 0;
}

int
azalia_generic_mixer_create_virtual(codec_t *this, int pdac, int padc)
{
	mixer_item_t *m;
	mixer_devinfo_t *d;
	convgroup_t *cgdac;
	convgroup_t *cgadc;
	int i, err, madc, mmaster;

	if (this->dacs.ngroups > 0)
		cgdac = &this->dacs.groups[0];
	if (this->adcs.ngroups > 0)
		cgadc = &this->adcs.groups[0];

	/* Clear mixer indexes, to make generic_mixer_fix_index happy */
	for (i = 0; i < this->nmixers; i++) {
		d = &this->mixers[i].devinfo;
		d->index = d->prev = d->next = 0;
	}

	madc = mmaster = -1;
	for (i = 0; i < this->nmixers && (mmaster < 0 || madc < 0); i++) {
		if (this->mixers[i].devinfo.type != AUDIO_MIXER_VALUE)
			continue;
		if (pdac >= 0 && this->mixers[i].nid == pdac)
			mmaster = i;
		if (mmaster < 0 && this->dacs.ngroups > 0 && cgdac->nconv > 0) {
			if (this->mixers[i].nid == cgdac->conv[0])
				mmaster = i;
		}
		if (padc >= 0 && this->mixers[i].nid == padc)
			madc = i;
		if (madc < 0 && this->adcs.ngroups > 0 && cgadc->nconv > 0) {
			if (this->mixers[i].nid == cgadc->conv[0])
				madc = i;
		}
	}

	if (mmaster >= 0) {
		err = azalia_generic_mixer_ensure_capacity(this, this->nmixers + 1);
		if (err)
			return err;
		m = &this->mixers[this->nmixers];
		d = &m->devinfo;
		memcpy(m, &this->mixers[mmaster], sizeof(*m));
		d->mixer_class = AZ_CLASS_OUTPUT;
		snprintf(d->label.name, sizeof(d->label.name), AudioNmaster);
		this->nmixers++;
	}

	if (madc >= 0) {
		err = azalia_generic_mixer_ensure_capacity(this, this->nmixers + 1);
		if (err)
			return err;
		m = &this->mixers[this->nmixers];
		d = &m->devinfo;
		memcpy(m, &this->mixers[madc], sizeof(*m));
		d->mixer_class = AZ_CLASS_RECORD;
		snprintf(d->label.name, sizeof(d->label.name), AudioNvolume);
		this->nmixers++;
	}

	azalia_generic_mixer_fix_indexes(this);

	return 0;
}

int
azalia_generic_mixer_autoinit(codec_t *this)
{
	azalia_generic_mixer_init(this);
	azalia_generic_mixer_create_virtual(this, -1, -1);
	azalia_generic_mixer_pin_sense(this);

	return 0;
}

int
azalia_generic_mixer_delete(codec_t *this)
{
	if (this->mixers == NULL)
		return 0;
	free(this->mixers, M_DEVBUF);
	this->mixers = NULL;
	return 0;
}

/**
 * @param mc	mc->type must be set by the caller before the call
 */
int
azalia_generic_mixer_get(const codec_t *this, nid_t nid, int target,
    mixer_ctrl_t *mc)
{
	uint32_t result;
	nid_t n;
	int i, err;

	/* inamp mute */
	if (IS_MI_TARGET_INAMP(target) && mc->type == AUDIO_MIXER_ENUM) {
		err = this->comresp(this, nid, CORB_GET_AMPLIFIER_GAIN_MUTE,
		    CORB_GAGM_INPUT | CORB_GAGM_LEFT |
		    MI_TARGET_INAMP(target), &result);
		if (err)
			return err;
		mc->un.ord = result & CORB_GAGM_MUTE ? 1 : 0;
	}

	/* inamp gain */
	else if (IS_MI_TARGET_INAMP(target) && mc->type == AUDIO_MIXER_VALUE) {
		err = this->comresp(this, nid, CORB_GET_AMPLIFIER_GAIN_MUTE,
		      CORB_GAGM_INPUT | CORB_GAGM_LEFT |
		      MI_TARGET_INAMP(target), &result);
		if (err)
			return err;
		mc->un.value.level[0] = azalia_generic_mixer_from_device_value(this,
		    nid, target, CORB_GAGM_GAIN(result));
		if (this->w[nid].type == COP_AWTYPE_AUDIO_SELECTOR ||
		    this->w[nid].type == COP_AWTYPE_AUDIO_MIXER) {
			n = this->w[nid].connections[MI_TARGET_INAMP(target)];
			if (!azalia_widget_enabled(this, n)) {
				DPRINTF(("%s: nid %2.2x invalid index %d\n",
				   __func__, nid,  MI_TARGET_INAMP(target)));
				n = nid;
			}
		} else
			n = nid;
		mc->un.value.num_channels = WIDGET_CHANNELS(&this->w[n]);
		if (mc->un.value.num_channels == 2) {
			err = this->comresp(this, nid,
			    CORB_GET_AMPLIFIER_GAIN_MUTE, CORB_GAGM_INPUT |
			    CORB_GAGM_RIGHT | MI_TARGET_INAMP(target),
			    &result);
			if (err)
				return err;
			mc->un.value.level[1] = azalia_generic_mixer_from_device_value
			    (this, nid, target, CORB_GAGM_GAIN(result));
		}
	}

	/* outamp mute */
	else if (target == MI_TARGET_OUTAMP && mc->type == AUDIO_MIXER_ENUM) {
		err = this->comresp(this, nid, CORB_GET_AMPLIFIER_GAIN_MUTE,
		    CORB_GAGM_OUTPUT | CORB_GAGM_LEFT | 0, &result);
		if (err)
			return err;
		mc->un.ord = result & CORB_GAGM_MUTE ? 1 : 0;
	}

	/* outamp gain */
	else if (target == MI_TARGET_OUTAMP && mc->type == AUDIO_MIXER_VALUE) {
		err = this->comresp(this, nid, CORB_GET_AMPLIFIER_GAIN_MUTE,
		      CORB_GAGM_OUTPUT | CORB_GAGM_LEFT | 0, &result);
		if (err)
			return err;
		mc->un.value.level[0] = azalia_generic_mixer_from_device_value(this,
		    nid, target, CORB_GAGM_GAIN(result));
		mc->un.value.num_channels = WIDGET_CHANNELS(&this->w[nid]);
		if (mc->un.value.num_channels == 2) {
			err = this->comresp(this, nid,
			    CORB_GET_AMPLIFIER_GAIN_MUTE,
			    CORB_GAGM_OUTPUT | CORB_GAGM_RIGHT | 0, &result);
			if (err)
				return err;
			mc->un.value.level[1] = azalia_generic_mixer_from_device_value
			    (this, nid, target, CORB_GAGM_GAIN(result));
		}
	}

	/* selection */
	else if (target == MI_TARGET_CONNLIST) {
		err = this->comresp(this, nid,
		    CORB_GET_CONNECTION_SELECT_CONTROL, 0, &result);
		if (err)
			return err;
		result = CORB_CSC_INDEX(result);
		if (!azalia_widget_enabled(this,
		    this->w[nid].connections[result]))
			mc->un.ord = -1;
		else
			mc->un.ord = result;
	}

	/* pin I/O */
	else if (target == MI_TARGET_PINDIR) {
		err = this->comresp(this, nid,
		    CORB_GET_PIN_WIDGET_CONTROL, 0, &result);
		if (err)
			return err;
		mc->un.ord = result & CORB_PWC_OUTPUT ? 1 : 0;
	}

	/* pin headphone-boost */
	else if (target == MI_TARGET_PINBOOST) {
		err = this->comresp(this, nid,
		    CORB_GET_PIN_WIDGET_CONTROL, 0, &result);
		if (err)
			return err;
		mc->un.ord = result & CORB_PWC_HEADPHONE ? 1 : 0;
	}

	/* DAC group selection */
	else if (target == MI_TARGET_DAC) {
		mc->un.ord = this->dacs.cur;
	}

	/* ADC selection */
	else if (target == MI_TARGET_ADC) {
		mc->un.ord = this->adcs.cur;
	}

	/* Volume knob */
	else if (target == MI_TARGET_VOLUME) {
		err = this->comresp(this, nid, CORB_GET_VOLUME_KNOB,
		    0, &result);
		if (err)
			return err;
		mc->un.value.level[0] = azalia_generic_mixer_from_device_value(this,
		    nid, target, CORB_VKNOB_VOLUME(result));
		mc->un.value.num_channels = 1;
	}

	/* S/PDIF */
	else if (target == MI_TARGET_SPDIF) {
		err = this->comresp(this, nid, CORB_GET_DIGITAL_CONTROL,
		    0, &result);
		if (err)
			return err;
		mc->un.mask = result & 0xff & ~(CORB_DCC_DIGEN | CORB_DCC_NAUDIO);
	} else if (target == MI_TARGET_SPDIF_CC) {
		err = this->comresp(this, nid, CORB_GET_DIGITAL_CONTROL,
		    0, &result);
		if (err)
			return err;
		mc->un.value.num_channels = 1;
		mc->un.value.level[0] = CORB_DCC_CC(result);
	}

	/* EAPD */
	else if (target == MI_TARGET_EAPD) {
		err = this->comresp(this, nid,
		    CORB_GET_EAPD_BTL_ENABLE, 0, &result);
		if (err)
			return err;
		mc->un.ord = result & CORB_EAPD_EAPD ? 1 : 0;
	}

	/* sense pin */
	else if (target == MI_TARGET_PINSENSE) {
		err = this->comresp(this, nid, CORB_GET_PIN_SENSE,
		    0, &result);
		if (err)
			return err;
		mc->un.ord = result & CORB_PS_PRESENCE ? 1 : 0;
	}

	/* mute set */
	else if (target == MI_TARGET_MUTESET && mc->type == AUDIO_MIXER_SET) {
		const widget_t *w;

		if (!azalia_widget_enabled(this, nid)) {
			DPRINTF(("%s: invalid muteset nid\n"));
			return EINVAL;
		}
		w = &this->w[nid];
		mc->un.mask = 0;
		for (i = 0; i < w->nconnections; i++) {
			if (!azalia_widget_enabled(this, w->connections[i]))
				continue;
			err = this->comresp(this, nid,
			    CORB_GET_AMPLIFIER_GAIN_MUTE,
			    CORB_GAGM_INPUT | CORB_GAGM_LEFT |
			    MI_TARGET_INAMP(i), &result);
			if (err)
				return err;
			mc->un.mask |= (result & CORB_GAGM_MUTE) ? 0 : (1 << i);
		}
	}

	else {
		printf("%s: internal error in %s: target=%x\n",
		    XNAME(this), __func__, target);
		return -1;
	}
	return 0;
}

int
azalia_generic_mixer_set(codec_t *this, nid_t nid, int target,
    const mixer_ctrl_t *mc)
{
	uint32_t result, value;
	int i, err;

	/* inamp mute */
	if (IS_MI_TARGET_INAMP(target) && mc->type == AUDIO_MIXER_ENUM) {
		/* We have to set stereo mute separately to keep each gain value. */
		err = this->comresp(this, nid, CORB_GET_AMPLIFIER_GAIN_MUTE,
		    CORB_GAGM_INPUT | CORB_GAGM_LEFT |
		    MI_TARGET_INAMP(target), &result);
		if (err)
			return err;
		value = CORB_AGM_INPUT | CORB_AGM_LEFT |
		    (target << CORB_AGM_INDEX_SHIFT) |
		    CORB_GAGM_GAIN(result);
		if (mc->un.ord)
			value |= CORB_AGM_MUTE;
		err = this->comresp(this, nid, CORB_SET_AMPLIFIER_GAIN_MUTE,
		    value, &result);
		if (err)
			return err;
		if (WIDGET_CHANNELS(&this->w[nid]) == 2) {
			err = this->comresp(this, nid,
			    CORB_GET_AMPLIFIER_GAIN_MUTE, CORB_GAGM_INPUT |
			    CORB_GAGM_RIGHT | MI_TARGET_INAMP(target),
			    &result);
			if (err)
				return err;
			value = CORB_AGM_INPUT | CORB_AGM_RIGHT |
			    (target << CORB_AGM_INDEX_SHIFT) |
			    CORB_GAGM_GAIN(result);
			if (mc->un.ord)
				value |= CORB_AGM_MUTE;
			err = this->comresp(this, nid,
			    CORB_SET_AMPLIFIER_GAIN_MUTE, value, &result);
			if (err)
				return err;
		}
	}

	/* inamp gain */
	else if (IS_MI_TARGET_INAMP(target) && mc->type == AUDIO_MIXER_VALUE) {
		if (mc->un.value.num_channels < 1)
			return EINVAL;
		err = this->comresp(this, nid, CORB_GET_AMPLIFIER_GAIN_MUTE,
		      CORB_GAGM_INPUT | CORB_GAGM_LEFT |
		      MI_TARGET_INAMP(target), &result);
		if (err)
			return err;
		value = azalia_generic_mixer_to_device_value(this, nid, target,
		    mc->un.value.level[0]);
		value = CORB_AGM_INPUT | CORB_AGM_LEFT |
		    (target << CORB_AGM_INDEX_SHIFT) |
		    (result & CORB_GAGM_MUTE ? CORB_AGM_MUTE : 0) |
		    (value & CORB_AGM_GAIN_MASK);
		err = this->comresp(this, nid, CORB_SET_AMPLIFIER_GAIN_MUTE,
		    value, &result);
		if (err)
			return err;
		if (mc->un.value.num_channels >= 2 &&
		    WIDGET_CHANNELS(&this->w[nid]) == 2) {
			err = this->comresp(this, nid,
			      CORB_GET_AMPLIFIER_GAIN_MUTE, CORB_GAGM_INPUT |
			      CORB_GAGM_RIGHT | MI_TARGET_INAMP(target),
			      &result);
			if (err)
				return err;
			value = azalia_generic_mixer_to_device_value(this, nid, target,
			    mc->un.value.level[1]);
			value = CORB_AGM_INPUT | CORB_AGM_RIGHT |
			    (target << CORB_AGM_INDEX_SHIFT) |
			    (result & CORB_GAGM_MUTE ? CORB_AGM_MUTE : 0) |
			    (value & CORB_AGM_GAIN_MASK);
			err = this->comresp(this, nid,
			    CORB_SET_AMPLIFIER_GAIN_MUTE, value, &result);
			if (err)
				return err;
		}
	}

	/* outamp mute */
	else if (target == MI_TARGET_OUTAMP && mc->type == AUDIO_MIXER_ENUM) {
		err = this->comresp(this, nid, CORB_GET_AMPLIFIER_GAIN_MUTE,
		    CORB_GAGM_OUTPUT | CORB_GAGM_LEFT, &result);
		if (err)
			return err;
		value = CORB_AGM_OUTPUT | CORB_AGM_LEFT | CORB_GAGM_GAIN(result);
		if (mc->un.ord)
			value |= CORB_AGM_MUTE;
		err = this->comresp(this, nid, CORB_SET_AMPLIFIER_GAIN_MUTE,
		    value, &result);
		if (err)
			return err;
		if (WIDGET_CHANNELS(&this->w[nid]) == 2) {
			err = this->comresp(this, nid,
			    CORB_GET_AMPLIFIER_GAIN_MUTE,
			    CORB_GAGM_OUTPUT | CORB_GAGM_RIGHT, &result);
			if (err)
				return err;
			value = CORB_AGM_OUTPUT | CORB_AGM_RIGHT |
			    CORB_GAGM_GAIN(result);
			if (mc->un.ord)
				value |= CORB_AGM_MUTE;
			err = this->comresp(this, nid,
			    CORB_SET_AMPLIFIER_GAIN_MUTE, value, &result);
			if (err)
				return err;
		}
	}

	/* outamp gain */
	else if (target == MI_TARGET_OUTAMP && mc->type == AUDIO_MIXER_VALUE) {
		if (mc->un.value.num_channels < 1)
			return EINVAL;
		err = this->comresp(this, nid, CORB_GET_AMPLIFIER_GAIN_MUTE,
		      CORB_GAGM_OUTPUT | CORB_GAGM_LEFT, &result);
		if (err)
			return err;
		value = azalia_generic_mixer_to_device_value(this, nid, target,
		    mc->un.value.level[0]);
		value = CORB_AGM_OUTPUT | CORB_AGM_LEFT |
		    (result & CORB_GAGM_MUTE ? CORB_AGM_MUTE : 0) |
		    (value & CORB_AGM_GAIN_MASK);
		err = this->comresp(this, nid, CORB_SET_AMPLIFIER_GAIN_MUTE,
		    value, &result);
		if (err)
			return err;
		if (mc->un.value.num_channels >= 2 &&
		    WIDGET_CHANNELS(&this->w[nid]) == 2) {
			err = this->comresp(this, nid,
			      CORB_GET_AMPLIFIER_GAIN_MUTE, CORB_GAGM_OUTPUT |
			      CORB_GAGM_RIGHT, &result);
			if (err)
				return err;
			value = azalia_generic_mixer_to_device_value(this, nid, target,
			    mc->un.value.level[1]);
			value = CORB_AGM_OUTPUT | CORB_AGM_RIGHT |
			    (result & CORB_GAGM_MUTE ? CORB_AGM_MUTE : 0) |
			    (value & CORB_AGM_GAIN_MASK);
			err = this->comresp(this, nid,
			    CORB_SET_AMPLIFIER_GAIN_MUTE, value, &result);
			if (err)
				return err;
		}
	}

	/* selection */
	else if (target == MI_TARGET_CONNLIST) {
		if (mc->un.ord < 0 ||
		    mc->un.ord >= this->w[nid].nconnections ||
		    !azalia_widget_enabled(this,
		    this->w[nid].connections[mc->un.ord]))
			return EINVAL;
		err = this->comresp(this, nid,
		    CORB_SET_CONNECTION_SELECT_CONTROL, mc->un.ord, &result);
		if (err)
			return err;
	}

	/* pin I/O */
	else if (target == MI_TARGET_PINDIR) {
		if (mc->un.ord >= 2)
			return EINVAL;
		err = this->comresp(this, nid,
		    CORB_GET_PIN_WIDGET_CONTROL, 0, &result);
		if (err)
			return err;
		if (mc->un.ord == 0) {
			result &= ~CORB_PWC_OUTPUT;
			result |= CORB_PWC_INPUT;
		} else {
			result &= ~CORB_PWC_INPUT;
			result |= CORB_PWC_OUTPUT;
		}
		err = this->comresp(this, nid,
		    CORB_SET_PIN_WIDGET_CONTROL, result, &result);
		if (err)
			return err;
	}

	/* pin headphone-boost */
	else if (target == MI_TARGET_PINBOOST) {
		if (mc->un.ord >= 2)
			return EINVAL;
		err = this->comresp(this, nid,
		    CORB_GET_PIN_WIDGET_CONTROL, 0, &result);
		if (err)
			return err;
		if (mc->un.ord == 0) {
			result &= ~CORB_PWC_HEADPHONE;
		} else {
			result |= CORB_PWC_HEADPHONE;
		}
		err = this->comresp(this, nid,
		    CORB_SET_PIN_WIDGET_CONTROL, result, &result);
		if (err)
			return err;
	}

	/* DAC group selection */
	else if (target == MI_TARGET_DAC) {
		if (this->running)
			return EBUSY;
		if (mc->un.ord >= this->dacs.ngroups)
			return EINVAL;
		if (mc->un.ord != this->dacs.cur)
			return azalia_codec_construct_format(this,
			    mc->un.ord, this->adcs.cur);
		else
			return 0;
	}

	/* ADC selection */
	else if (target == MI_TARGET_ADC) {
		if (this->running)
			return EBUSY;
		if (mc->un.ord >= this->adcs.ngroups)
			return EINVAL;
		if (mc->un.ord != this->adcs.cur)
			return azalia_codec_construct_format(this,
			    this->dacs.cur, mc->un.ord);
		else
			return 0;
	}

	/* Volume knob */
	else if (target == MI_TARGET_VOLUME) {
		if (mc->un.value.num_channels != 1)
			return EINVAL;
		value = azalia_generic_mixer_to_device_value(this, nid, target,
		     mc->un.value.level[0]) | CORB_VKNOB_DIRECT;
		err = this->comresp(this, nid, CORB_SET_VOLUME_KNOB,
		   value, &result);
		if (err)
			return err;
	}

	/* S/PDIF */
	else if (target == MI_TARGET_SPDIF) {
		err = this->comresp(this, nid, CORB_GET_DIGITAL_CONTROL,
		    0, &result);
		result &= CORB_DCC_DIGEN | CORB_DCC_NAUDIO;
		result |= mc->un.mask & 0xff & ~CORB_DCC_DIGEN;
		err = this->comresp(this, nid, CORB_SET_DIGITAL_CONTROL_L,
		    result, NULL);
		if (err)
			return err;
	} else if (target == MI_TARGET_SPDIF_CC) {
		if (mc->un.value.num_channels != 1)
			return EINVAL;
		if (mc->un.value.level[0] > 127)
			return EINVAL;
		err = this->comresp(this, nid, CORB_SET_DIGITAL_CONTROL_H,
		    mc->un.value.level[0], NULL);
		if (err)
			return err;
	}

	/* EAPD */
	else if (target == MI_TARGET_EAPD) {
		if (mc->un.ord >= 2)
			return EINVAL;
		err = this->comresp(this, nid,
		    CORB_GET_EAPD_BTL_ENABLE, 0, &result);
		if (err)
			return err;
		result &= 0xff;
		if (mc->un.ord == 0) {
			result &= ~CORB_EAPD_EAPD;
		} else {
			result |= CORB_EAPD_EAPD;
		}
		err = this->comresp(this, nid,
		    CORB_SET_EAPD_BTL_ENABLE, result, &result);
		if (err)
			return err;
	}

	else if (target == MI_TARGET_PINSENSE) {
		/* do nothing, control is read only */
	}

	else if (target == MI_TARGET_MUTESET && mc->type == AUDIO_MIXER_SET) {
		const widget_t *w;

		if (!azalia_widget_enabled(this, nid)) {
			DPRINTF(("%s: invalid muteset nid\n"));
			return EINVAL;
		}
		w = &this->w[nid];
		for (i = 0; i < w->nconnections; i++) {
			if (!azalia_widget_enabled(this, w->connections[i]))
				continue;

			/* We have to set stereo mute separately
			 * to keep each gain value.
			 */
			err = this->comresp(this, nid,
			    CORB_GET_AMPLIFIER_GAIN_MUTE,
			    CORB_GAGM_INPUT | CORB_GAGM_LEFT |
			    MI_TARGET_INAMP(i), &result);
			if (err)
				return err;
			value = CORB_AGM_INPUT | CORB_AGM_LEFT |
			    (i << CORB_AGM_INDEX_SHIFT) |
			    CORB_GAGM_GAIN(result);
			if ((mc->un.mask & (1 << i)) == 0)
				value |= CORB_AGM_MUTE;
			err = this->comresp(this, nid,
			    CORB_SET_AMPLIFIER_GAIN_MUTE, value, &result);
			if (err)
				return err;

			if (WIDGET_CHANNELS(w) == 2) {
				err = this->comresp(this, nid,
				    CORB_GET_AMPLIFIER_GAIN_MUTE,
				    CORB_GAGM_INPUT | CORB_GAGM_RIGHT |
				    MI_TARGET_INAMP(i), &result);
				if (err)
					return err;
				value = CORB_AGM_INPUT | CORB_AGM_RIGHT |
				    (i << CORB_AGM_INDEX_SHIFT) |
				    CORB_GAGM_GAIN(result);
				if ((mc->un.mask & (1 << i)) == 0)
					value |= CORB_AGM_MUTE;
				err = this->comresp(this, nid,
				    CORB_SET_AMPLIFIER_GAIN_MUTE,
				    value, &result);
				if (err)
					return err;
			}
		}
	}

	else {
		printf("%s: internal error in %s: target=%x\n",
		    XNAME(this), __func__, target);
		return -1;
	}
	return 0;
}

u_char
azalia_generic_mixer_from_device_value(const codec_t *this, nid_t nid, int target,
    uint32_t dv)
{
	uint32_t dmax;

	if (IS_MI_TARGET_INAMP(target))
		dmax = COP_AMPCAP_NUMSTEPS(this->w[nid].inamp_cap);
	else if (target == MI_TARGET_OUTAMP)
		dmax = COP_AMPCAP_NUMSTEPS(this->w[nid].outamp_cap);
	else if (target == MI_TARGET_VOLUME)
		dmax = COP_VKCAP_NUMSTEPS(this->w[nid].d.volume.cap);
	else {
		printf("unknown target: %d\n", target);
		dmax = 255;
	}
	if (dv <= 0 || dmax == 0)
		return AUDIO_MIN_GAIN;
	if (dv >= dmax)
		return AUDIO_MAX_GAIN - AUDIO_MAX_GAIN % dmax;
	return dv * (AUDIO_MAX_GAIN - AUDIO_MAX_GAIN % dmax) / dmax;
}

uint32_t
azalia_generic_mixer_to_device_value(const codec_t *this, nid_t nid, int target,
    u_char uv)
{
	uint32_t dmax;

	if (IS_MI_TARGET_INAMP(target))
		dmax = COP_AMPCAP_NUMSTEPS(this->w[nid].inamp_cap);
	else if (target == MI_TARGET_OUTAMP)
		dmax = COP_AMPCAP_NUMSTEPS(this->w[nid].outamp_cap);
	else if (target == MI_TARGET_VOLUME)
		dmax = COP_VKCAP_NUMSTEPS(this->w[nid].d.volume.cap);
	else {
		printf("unknown target: %d\n", target);
		dmax = 255;
	}
	if (uv <= AUDIO_MIN_GAIN || dmax == 0)
		return 0;
	if (uv >= AUDIO_MAX_GAIN - AUDIO_MAX_GAIN % dmax)
		return dmax;
	return uv * dmax / (AUDIO_MAX_GAIN - AUDIO_MAX_GAIN % dmax);
}

int
azalia_generic_set_port(codec_t *this, mixer_ctrl_t *mc)
{
	const mixer_item_t *m;

	if (mc->dev >= this->nmixers)
		return ENXIO;
	m = &this->mixers[mc->dev];
	if (mc->type != m->devinfo.type)
		return EINVAL;
	if (mc->type == AUDIO_MIXER_CLASS)
		return 0;	/* nothing to do */
	return azalia_generic_mixer_set(this, m->nid, m->target, mc);
}

int
azalia_generic_get_port(codec_t *this, mixer_ctrl_t *mc)
{
	const mixer_item_t *m;

	if (mc->dev >= this->nmixers)
		return ENXIO;
	m = &this->mixers[mc->dev];
	mc->type = m->devinfo.type;
	if (mc->type == AUDIO_MIXER_CLASS)
		return 0;	/* nothing to do */
	return azalia_generic_mixer_get(this, m->nid, m->target, mc);
}

int
azalia_gpio_unmute(codec_t *this, int pin)
{
	uint32_t data, mask, dir;

	this->comresp(this, this->audiofunc, CORB_GET_GPIO_DATA, 0, &data);
	this->comresp(this, this->audiofunc, CORB_GET_GPIO_ENABLE_MASK, 0, &mask);
	this->comresp(this, this->audiofunc, CORB_GET_GPIO_DIRECTION, 0, &dir);

	data |= 1 << pin;
	mask |= 1 << pin;
	dir |= 1 << pin;

	this->comresp(this, this->audiofunc, CORB_SET_GPIO_ENABLE_MASK, mask, NULL);
	this->comresp(this, this->audiofunc, CORB_SET_GPIO_DIRECTION, dir, NULL);
	DELAY(1000);
	this->comresp(this, this->audiofunc, CORB_SET_GPIO_DATA, data, NULL);

	return 0;
}

void
azalia_pin_config_ov(widget_t *w, int mask, int val)
{
	int bits, offset;

	switch (mask) {
	case CORB_CD_DEVICE_MASK:
		bits = CORB_CD_DEVICE_BITS;
		offset = CORB_CD_DEVICE_OFFSET;
		break;
	case CORB_CD_PORT_MASK:
		bits = CORB_CD_PORT_BITS;
		offset = CORB_CD_PORT_OFFSET;
		break;
	default:
		return;
	}
	val &= bits;
	w->d.pin.config &= ~(mask);
	w->d.pin.config |= val << offset;
	if (mask == CORB_CD_DEVICE_MASK)
		w->d.pin.device = val;
}


/* ----------------------------------------------------------------
 * Realtek ALC260
 *
 * Fujitsu LOOX T70M/T
 *	Internal Speaker: 0x10
 *	Front Headphone: 0x14
 *	Front mic: 0x12
 * ---------------------------------------------------------------- */

int
azalia_alc260_mixer_init(codec_t *this)
{
	azalia_generic_mixer_init(this);
	azalia_generic_mixer_create_virtual(this, 0x08, -1);
	azalia_generic_mixer_pin_sense(this);

	return 0;
}

/* ----------------------------------------------------------------
 * Realtek ALC88x mixer init - volume control is on the mixer
 * instead of the DAC.
 * ---------------------------------------------------------------- */

int
azalia_alc88x_mixer_init(codec_t *this)
{
	azalia_generic_mixer_init(this);
	azalia_generic_mixer_create_virtual(this, 0x0c, -1);
	azalia_generic_mixer_pin_sense(this);

	return 0;
}

