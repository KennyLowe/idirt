#ifndef _MUDMACROS_H
#define _MUDMACROS_H

#include "macros.h"

#define is_in_game(C)           ((C) >= 0 && (C) < numchars \
				 && !EMPTY(pname(C)) && ploc(C) < 0 \
				 && ((C) >= max_players || players[C].iamon))

#define is_conn(C)		players[C].is_conn
#define is_ban(C)		players[C].host_ban
#define	is_userban(C)		players[C].user_ban

#define his_or_her(C)		(psex(C) ? "her" : "his")
#define him_or_her(C)           (psex(C) ? "her" : "him")
#define he_or_she(C)            (psex(C) ? "She" : "He")

#define is_aliased(C)		(players[C].aliased || players[C].polymorphed >= 0)

/* Players and Mobiles */

#define pagg(C)                 ublock[C].pagg
#define setpagg(C, V)           (pagg(C) = (V))
#define pagg_reset(C)           ublock[C].pagg_reset

#define pspeed(C)               ublock[C].pspeed
#define setpspeed(C, V)         (pspeed(C) = (V))
#define pspeed_reset(C)         ublock[C].pspeed_reset

#define pdam(C)                 ublock[C].pdam
#define setpdam(C, V)           (pdam(C) = (V))
#define pdam_reset(C)           ublock[C].pdam_reset

#define parmor(C)               ublock[C].parmor
#define setparmor(C, V)         (parmor(C) = (V))
#define parmor_reset(C)         ublock[C].parmor_reset

#define pscore(C)		(ublock[C].pscore)
#define setpscore(C, V)		(pscore(C) = (V))

#define pfighting(C)		(ublock[C].pfighting)

#define phelping(C)		(ublock[C].phelping)
#define setphelping(C, V)	(phelping(C) = (V))

#define psitting(C)		(ublock[C].psitting)
#define setpsitting(C, V)	(psitting(C) = (V))

#define pwpn(C)			(ublock[C].pweapon)
#define setpwpn(C, V)		(pwpn(C) = (V))

#define plev(C)			(ublock[C].plev)
#define setplev(C, V)		(plev(C) = (V))
#define plev_reset(C)		(ublock[C].plev_reset)

#define pmobile(C)              ((C) >= max_players)

#define pftxt(C)                (ublock[C].pftxt)
#define pexam(C)                (ublock[C].p_exam)

#define pflags(C)		(ublock[C].pflags)
#define pclrflg(C, V)		dclr_bit(&pflags(C),(V))
#define psetflg(C, V)		dset_bit(&pflags(C),(V))
#define ptstflg(C, V)		dtst_bit(&pflags(C),(V))
#define setpflags(C, V)		(pflags(C) = (V))
#define setpflgu(C, V)		(pflags(C).u = (V))
#define setpflgh(C, V)		(pflags(C).h = (V))
#define setpflgl(C, V)		(pflags(C).l = (V))
#define pflags_reset(C)		(ublock[C].pflags_reset)

#define pmask(C)		(ublock[C].pmask)
#define pclrmsk(C, V)		dclr_bit(&pmask(C),(V))
#define psetmsk(C, V)		dset_bit(&pmask(C),(V))
#define ptstmsk(C, V)		dtst_bit(&pmask(C),(V))
#define setpmask(C, V)		(pmask(C) = (V))
#define setpmsku(C, V)		(pmask(C).u = (V))
#define setpmskh(C, V)		(pmask(C).h = (V))
#define setpmskl(C, V)		(pmask(C).l = (V))

#define mflags(C)		(ublock[C].pmflags)
#define mclrflg(C, V)		clr_bit(&mflags(C),(V))
#define msetflg(C, V)		set_bit(&mflags(C),(V))
#define mtstflg(C, V)		tst_bit(&mflags(C),(V))
#define setmflags(C, V)		(mflags(C) = (V))
#define setmflgh(C, V)		(mflags(C).h = (V))
#define setmflgl(C, V)		(mflags(C).l = (V))
#define mflags_reset(C)		(ublock[C].pmflags_reset)

#define sflags(C)               (ublock[C].psflags)
#define sclrflg(C, V)           clr_bit(&sflags(C),(V))
#define ssetflg(C, V)           set_bit(&sflags(C),(V))
#define ststflg(C, V)           tst_bit(&sflags(C),(V))
#define setsflags(C, V)         (sflags(C) = (V))
#define setsflgh(C, V)          (sflags(C).h = (V))
#define setsflgl(C, V)          (sflags(C).l = (V))
#define sflags_reset(C)         (ublock[C].psflags_reset)

#define nflags(C)               (ublock[C].pnflags)
#define nclrflg(C, V)           xclrbit(nflags(C),(V))
#define nsetflg(C, V)           xsetbit(nflags(C),(V))
#define ntstflg(C, V)           xtstbit(nflags(C),(V))
#define setnflags(C, V)         (nflags(C) = (V))
#define nflags_reset(C)         (ublock[C].pnflags_reset)

#define eflags(C)               (ublock[C].peflags)
#define eclrflg(C, V)           xclrbit(eflags(C),(V))
#define esetflg(C, V)           xsetbit(eflags(C),(V))
#define etstflg(C, V)           xtstbit(eflags(C),(V))
#define seteflags(C, V)         (eflags(C) = (V))
#define eflags_reset(C)         (ublock[C].peflags_reset)

#define plang(C)		ublock[C].planguage 
#define setplang(C, V)		(plang(C) = (V))
#define is_eng(C)		(plang(C) == NFL_ENGLISH)

#define qflags(C)		(ublock[C].pquests)
#define qclrflg(C, V)		xclrbit(qflags(C),(V))
#define qsetflg(C, V)		xsetbit(qflags(C),(V))
#define qtstflg(C, V)		xtstbit(qflags(C),(V))
#define setqflags(C, V)		(qflags(C) = (V))

#define qdclrflg(C)             xclrbit(qdone, (C))
#define qdsetflg(C)             xsetbit(qdone, (C))
#define qdtstflg(C)             xtstbit(qdone, (C))
#define setqdflags(C)           (qdone = (C))
 
#define psex(C)			bits(sflags(C).l,1)

#define pvis(C)			(ublock[C].pvis)
#define setpvis(C, V)		(pvis(C) = (V))
#define pvis_reset(C)		(ublock[C].pvis_reset)

#define pstr(C)			(ublock[C].pstr)
#define pmaxstrength(L)		(50 + 8*(L))
#define setpstr(C, V)		(pstr(C) = (V))
#define pstr_reset(C)		(ublock[C].pstr_reset)

#define ploc(C)			(ublock[C].ploc)
#define ploc_reset(C)           (ublock[C].ploc_reset)

#define phome(C)                (ublock[C].phome)     
#define setphome(C, V)          (phome(C) = (V))

#define pname(C)                (ublock[C].pname)
#define setpname(C, V)          (strcpy(pname(C), (V)))
#define pname_reset(C)          (ublock[C].pname_reset)

#define ptitle(C)               players[C].ptitle
#define setptitle(C, V)         (strcpy(ptitle(C), (V)))

#define pwimpy(C)               ublock[C].pwimpy
#define pwimpy_reset(C)         ublock[C].pwimpy_reset
#define setpwimpy(C, V)         (pwimpy(C) = (V))

#define pnum(C)                 ublock[C].pnum
#define ptemporary(C)           ublock[C].temporary
#define pzone(C)                ublock[C].zone
#define mob_id(C)               ublock[C].id
#define ppermanent(C)           ((C) < num_const_chars)
#define plast_cmd(C)            (players[C].last_cmd)
#define prlast_cmd(C)           (players[C].rlast_cmd)
#define plogged_on(C)           (players[C].logged_on)
#define powner(C)               zname(pzone(C))

#define pinv(C)                 (&ublock[C].objects)
#define pfirst_obj(C)           first_int(pinv(C))
#define pnext_obj(C)            next_int(pinv(C))
#define pnumobs(C)              set_size(pinv(C))
#define pobj_nr(N, C)           int_number((N), pinv(C))

#define pkilled(C)              (players[C].pkilled)
#define setpkilled(C, V)        (pkilled(C) = (V))

#define pdied(C)                (players[C].pdied)
#define setpdied(C, V)          (pdied(C) = (V))

#define pchannel(C)             (players[C].pchannel)
#define setpchannel(C, V)       (pchannel(C) = (V))

#define pmagic(C)               (players[C].pmagic)
#define pmaxmagic(L)            L > 0 ? (12 + 11*(L)) : 30
#define setpmagic(C, V)         (pmagic(C) = (V))

#define pspeech(C, V)           ublock[C].pspeech[V]
#define setpspeech(C, V, L)     (strcpy(pspeech(C, V), (L)))

#define pchance(C)              ublock[C].pchance
#define setpchance(C, V)        (pchance(C) = (V))

#define phandler(C)             players[C].inp_handler->inp_handler
#define plastcom(C)             players[C].prev_com

#define pconv(C)                players[C].pconverse
#define setpconv(C, V)          (pconv(C) = (V))

#define ppager(C)               (players[C].pager.len)
#define setppager(C, V)         (ppager(C) = (V))

#define islogged(C)             (players[C].logged)
#define pfollow(C)		players[C].i_follow

/* Locations */

#define convroom(L)		(-(L)-1)

#define lpermanent(L)           ((L) > convroom(num_const_locs))
#define xlflags(L)              room_data[convroom(L)].r_flags
#define xlflags_reset(L)        room_data[convroom(L)].r_flags_reset
#define lclrflg(L, V)           clr_bit(&xlflags(L),(V))
#define lsetflg(L, V)           set_bit(&xlflags(L),(V))
#define ltstflg(L, V)           tst_bit(&xlflags(L),(V))
#define loc_id(L)               room_data[convroom(L)].id
#define lexit(L, E)             room_data[convroom(L)].r_exit[E]
#define lexit_reset(L, E)       room_data[convroom(L)].r_exit_reset[E]
#define lshort(L)               room_data[convroom(L)].r_short
#define llong(L)                room_data[convroom(L)].r_long
#define ltemporary(L)           room_data[convroom(L)].temporary
#define lowner(L)               zname(lzone(L))

#define lexits_to_me(L)         (&room_data[convroom(L)].exits_to_me)
#define ltouched(L)             (room_data[convroom(L)].touched)
#define lzone(L)                (room_data[convroom(L)].zone)

#define linv(L)                 (&room_data[convroom(L)].objects)
#define lfirst_obj(C)           first_int(linv(C))
#define lnext_obj(C)            next_int(linv(C))
#define lnumobs(C)              set_size(linv(C))
#define lobj_nr(N, L)           int_number((N), linv(L))

#define lmobs(L)                (&room_data[convroom(L)].mobiles)
#define lfirst_mob(C)           first_int(lmobs(C))
#define lnext_mob(C)            next_int(lmobs(C))
#define lnumchars(C)            set_size(lmobs(C))
#define lmob_nr(N, L)           int_number((N), lmobs(L))

/* Objects */

#define oarmor(O)	        objects[O].oarmor
#define oarmor_reset(O)	        objects[O].oarmor_reset
#define odamage(O)		objects[O].odamage
#define odamage_reset(O)	objects[O].odamage_reset
#define osetarmor(O,V)		(oarmor(O) = (V))
#define osetdamage(O,V)		(odamage(O) = (V))

#define ovis(O)                 (objects[O].ovis)
#define osetvis(O, V)           (ovis(O) = (V))
#define ovis_reset(O)           (objects[O].ovis_reset)

#define obits(O)		(objects[O].oflags)
#define obits_reset(O)		(objects[O].oflags_reset)
#define otstbit(O, V)		dtst_bit(&obits(O),(V))
#define oclrbit(O, V)		dclr_bit(&obits(O),(V))
#define osetbit(O, V)		dset_bit(&obits(O),(V))

#define oloc(O)			objects[O].oloc
#define oloc_reset(O)		objects[O].oloc_reset

#define state(O)		objects[O].ostate
#define state_reset(O)		objects[O].ostate_reset
#define ocarrf(O)		objects[O].ocarrf
#define ocarrf_reset(O)		objects[O].ocarrf_reset
#define setcarrf(O,C)           (ocarrf(O) = (C))
#define olongt(O, V)		(objects[O].odesc[V])
#define omaxstate(O)		(objects[O].omaxstate)
#define oaltname(O)		(objects[O].oaltname)

#define osize(O)		(objects[O].osize)
#define osize_reset(O)		(objects[O].osize_reset)

#define osetsize(O, V)          (osize(O) = (V))
#define oflannel(O)		otstbit(O,OFL_NOGET)

#define obaseval(O)		(objects[O].ovalue)
#define ovalue_reset(O) 	(objects[O].ovalue_reset)
#define osetbaseval(O, V)       (obaseval(O) = (V))

#define oname(O)                objects[O].oname
#define ospare(O)               (otstbit(O, OFL_DESTROYED) ? -1 : 0)
#define oexamine(O)             objects[O].oexamine
#define oexam_text(O)           objects[O].oexam_text

#define opermanent(O)           ((O) < num_const_obs)
#define otemporary(O)           objects[O].temporary
#define olinked(O)              objects[O].linked
#define ozone(O)                objects[O].zone
#define oowner(O)               zname(ozone(O))
#define obj_id(O)               objects[O].id
#define onum(O)                 objects[O].onum

#define oinv(C)                 (&objects[C].objects)
#define ofirst_obj(C)           first_int(oinv(C))
#define onext_obj(C)            next_int(oinv(C))
#define onumobs(C)              set_size(oinv(C))
#define oobj_nr(N, C)           int_number((N), oinv(C))

/* Zones */

#define zname(Z)                zoname[Z].z_name

#define zmaxlocs(Z)             zoname[Z].maxlocs
#define zmaxmobs(Z)             zoname[Z].maxmobs
#define zmaxobjs(Z)             zoname[Z].maxobjs

#define zlocs(Z)                &zoname[Z].locations
#define zmobs(Z)                &zoname[Z].mobiles
#define zobjs(Z)                &zoname[Z].objects

#define ztemporary(Z)           zoname[Z].temporary
#define zpermanent(Z)           ((Z) >= 0 && (Z) < num_const_zon)

#define zfirst_obj(C)           first_int(zobjs(C))
#define znext_obj(C)            next_int(zobjs(C))

#define zfirst_mob(C)           first_int(zmobs(C))
#define znext_mob(C)            next_int(zmobs(C))

#define zfirst_loc(C)           first_int(zlocs(C))
#define znext_loc(C)            next_int(zlocs(C))

#define zadd_obj(O, Z)          add_int((O), zobjs(Z))
#define zadd_mob(O, Z)          add_int((O), zmobs(Z))
#define zadd_loc(O, Z)          add_int((O), zlocs(Z))

#define zremove_obj(O, Z)       remove_int((O), zobjs(Z))
#define zremove_mob(O, Z)       remove_int((O), zmobs(Z))
#define zremove_loc(O, Z)       remove_int((O), zlocs(Z))

#define znumobs(O)              set_size(zobjs(O))
#define znumloc(O)              set_size(zlocs(O))
#define znumchars(O)            set_size(zmobs(O))

#define zobj_nr(N, Z)           int_number((N), zobjs(Z))
#define zmob_nr(N, Z)           int_number((N), zmobs(Z))
#define zloc_nr(N, Z)           int_number((N), zlocs(Z))

/* General */

#define first_obj(C)            first_int(C)
#define next_obj(C)             next_int(C)
#define first_mob(C)            first_int(C)
#define next_mob(C)             next_int(C)
#define first_loc(C)            first_int(C)
#define next_loc(C)             next_int(C)

#endif
