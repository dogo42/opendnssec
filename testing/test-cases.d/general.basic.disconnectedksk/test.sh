#!/usr/bin/env bash

if [ -n "$HAVE_MYSQL" ]; then
	return 0
fi
PATH=$INSTALL_ROOT/bin:$INSTALL_ROOT/sbin:$PATH
echo $PATH
export PATH

log_this 01 cp kasp.xml zonelist.xml $INSTALL_ROOT/etc/opendnssec/. &&
log_this 02 cp conf-bunker.xml $INSTALL_ROOT/etc/opendnssec/conf.xml &&
log_this 03 cp softhsm-bunker.conf $INSTALL_ROOT/etc/softhsm.conf &&
log_this 04 apply_parameter "INSTALL_ROOT" "$INSTALL_ROOT" "$INSTALL_ROOT/etc/softhsm.conf" &&
log_this 04 apply_parameter "INSTALL_ROOT" "$INSTALL_ROOT" "$INSTALL_ROOT/etc/opendnssec/conf.xml" &&
log_this 05 apply_parameter "SOFTHSM_MODULE" "$SOFTHSM_MODULE" "$INSTALL_ROOT/etc/opendnssec/conf.xml" &&
log_this 06 rm -f "$INSTALL_ROOT/var/opendnssec/enforcer/zones.xml" &&
log_this 07 softhsm --init-token --pin 1234 --slot 0 --label OpenDNSSEC --so-pin 1234 &&
log_this 08 softhsm --init-token --pin 1234 --slot 1 --label KSKs --so-pin 1234 &&
log_this 09 softhsm --init-token --pin 1234 --slot 2 --label ZSKs --so-pin 1234 &&
echo 'y' | log_this 10 ods-enforcer-db-setup &&
log_this 11 ods-enforcerd --set-time 2017-01-01-00:00:00 &&
# sleep 15 &&    unsure whether this is really needed.
log_this 12 ods-enforcer policy import &&
log_this 13 ods-enforcer zone add -z xx &&
log_this 14 ods_enforcer_idle &&
log_this 15 ods-enforcer time leap &&
log_this 16 ods-enforcer time leap &&
log_this 17 ods-enforcer time leap &&
log_this 18 ods-enforcer time leap &&
( log_this 16 ods-enforcer signconf || echo "signconf unjustly failing" ) &&
log_this 19 ods_enforcer_idle &&
log_this 20 ods-enforcer stop &&
log_this 21 ods-signerd --set-time 2017-01-01-00:00:00 &&
(syslog_waitfor_count 90 2 'ods-signerd: .*\[STATS\] xx ' ||
 (ods-signer sign --all &&
  syslog_waitfor_count 90 2 'ods-signerd: .*\[STATS\] xx ')) &&
log_this 22 ods-signer stop &&
sleep 15 &&
log_this 23 perl sneakernet.pl $INSTALL_ROOT/var/opendnssec/signconf/xx.xml $INSTALL_ROOT/var/opendnssec/signer/xx.backup2 &&
log_this 24 rm -f $INSTALL_ROOT/var/opendnssec/signer/* $INSTALL_ROOT/var/opendnssec/signed/* &&
log_this 25 ods-signerd --set-time 2017-02-01-00:00:00 &&
(syslog_waitfor_count 90 2 'ods-signerd: .*\[STATS\] xx ' ||
 (ods-signer sign --all &&
  syslog_waitfor_count 90 2 'ods-signerd: .*\[STATS\] xx ')) &&
log_this 26 ods-signer stop &&
sleep 15 &&

return 0

echo
echo "************ERROR******************"
echo
ods_kill
return 1
