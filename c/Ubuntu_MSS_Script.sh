#!/bin/bash

if [[ ! "x$1" =~ "@" ]]; then
	echo "usage: $0 email"
	exit 1
fi

#
# general
#
LDE_DOWNLOAD_PATH="http://shilc109.sh.intel.com/repository"
DNSDOMAIN="sh.intel.com"
export DEBIAN_FRONTEND=noninteractive 

echo 'Acquire::http::proxy "http://10.239.120.36:911";' >> /etc/apt/apt.conf
apt-get -y update
mkdir -p /etc/intel

#
# configure passwords policy
#
PASS_EXPIRATION_DAYS=90

# set PASS_MAX_DAYS for existing users
UID_MIN=`grep -e '^UID_MIN' /etc/login.defs | sed 's/\s\+/\t/g' | cut -f2`
while read i; do
	user=`echo $i | cut -d: -f1`
	user_id=`echo $i | cut -d: -f3`
	if [[ $user_id -ge $UID_MIN && $user != "nobody" ]]; then
		chage -M $PASS_EXPIRATION_DAYS -W $PASS_EXPIRATION_DAYS $user
	fi
done < /etc/passwd

# set PASS_MAX_DAYS for new users
sed -i "s/^[\s#]*PASS_MAX_DAYS\s\+[0-9]\+/PASS_MAX_DAYS ${PASS_EXPIRATION_DAYS}/" /etc/login.defs
sed -i "s/^[\s#]*PASS_WARN_AGE\s\+[0-9]\+/PASS_WARN_AGE ${PASS_EXPIRATION_DAYS}/" /etc/login.defs

#
# install bigfix
#
BIGFIX_SITE="SHZ-Zizhu"
export BFREPO=${LDE_DOWNLOAD_PATH}/lde_3dparty/BES/                       
wget -q -O /etc/intel/bigfix_install.sh  ${BFREPO}bigfix_install.sh       
chmod +x /etc/intel/bigfix_install.sh                                     
/etc/intel/bigfix_install.sh --site ${BIGFIX_SITE} --lab SW_LAB --category ALL

#
# setup autoupdates
#
EMAIL=$1
debconf-set-selections <<EOF
unattended-upgrades     unattended-upgrades/enable_auto_updates boolean true
EOF
apt-get -y install unattended-upgrades ssmtp

# configure ssmtp
DR_HOST=mailhost.$DNSDOMAIN
mailConf=`cat /etc/ssmtp/ssmtp.conf | grep mailhub`
sed -i.orig -e s/$mailConf/"mailhub="${DR_HOST}/g /etc/ssmtp/ssmtp.conf

# configure unattended-upgrades
dpkg-reconfigure -f noninteractive unattended-upgrades

# enable all updates
sed -i '/${distro_id}:${distro_codename}-/ s/^[\s /]*/ /' /etc/apt/apt.conf.d/50unattended-upgrades
# uncomment
sed -i "/Unattended-Upgrade::InstallOnShutdown/ s/^[\s /]*//" /etc/apt/apt.conf.d/50unattended-upgrades
# install updates without shutdown
sed -i "/^Unattended-Upgrade::InstallOnShutdown/ s/true/false/" /etc/apt/apt.conf.d/50unattended-upgrades
# enable mail notifications
sed -i "/^[/ ]*Unattended-Upgrade::Mail/ s/^[ /]*//" /etc/apt/apt.conf.d/50unattended-upgrades
sed -i "/^Unattended-Upgrade::Mail / s/root/${EMAIL}/" /etc/apt/apt.conf.d/50unattended-upgrades
sed -i "/^Unattended-Upgrade::MailOnlyOnError / s/true/false/" /etc/apt/apt.conf.d/50unattended-upgrades

#
# install something
#
cd /etc/intel
wget -O rsyslog.conf -c ${LDE_DOWNLOAD_PATH}/lde_3dparty/debian/other/rsyslog.conf.tpl
cp -f /etc/rsyslog.conf /etc/rsyslog.conf.backup
cp -f /etc/intel/rsyslog.conf /etc/rsyslog.conf
service rsyslog restart

