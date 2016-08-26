#!/bin/ksh
#
#    Clax Installation Script 
#    version: 0.1
#
progname=$0
basedir=`dirname $0`

# Default options
username=`id|sed 's/).*$//g;s/^.*(//g'`
dir="/opt/clax"
port="11801"
inetd="inetd"
silent="no"
force="no"

function create_dir {
    if [ ! -d "$dir" ]
    then
      echo "Creating $dir"
      mkdir "$dir"
    elif [ $force = "no" ]
    then
      echo "$dir already exists.  Use force mode to overwrite"
    fi
}

function copy_binary {
    echo "Copying clax binary to $dir"
    cp $basedir/clax "$dir"
}

function create_ini {
    echo "Generating clax.ini file"
    cat <<EOI >$dir/clax.ini
root = $dir
log_file = $dir/clax.log

[ssl]
enabled = no
cert_file = $dir/clax.crt
key_file = $dir/clax.key
#entropy_file = $dir/entropy

#[http_basic_auth]
#username = clax
#password = password
EOI
}

function generate_services {
    echo "Generating /etc/services file"

    clax_service=`grep '^clax' /etc/services`

    if [ "$clax_service" ]
    then
      echo "Clax service already exists in /etc/services"
      sed 's/^clax.*//' /etc/services > $dir/services.clax
    else
      cp /etc/services $dir/services.clax
    fi
    echo "clax      $port/tcp" >> $dir/services.clax
}

function generate_inetd {
    echo "Generating inetd.clax file"
    echo "clax stream tcp nowait $username $dir/clax clax -l $dir/clax.log" > $dir/inetd.clax
    echo "Generating xinetd.clax file"
    cat >$dir/xinetd.clax <<EOX
service clax {
    disable = no
    id = clax
    type = UNLISTED
    socket_type = stream
    protocol = tcp
    port = $port
    wait = no
    user = clax
    server = $dir/clax
    server_args = -l $dir/clax.log
    instances = 5
}
EOX
}

function change_owner {
    echo "Changing $dir ownership to $username"
    chown -R $username $dir
}

function platform_config {
    cat >$dir/enable_clax_inetd_generic.sh <<GEN
cp /etc/services $dir/services.bak
cp $dir/services.clax /etc/services
cat $dir/inetd.clax >> /etc/inetd.conf

pid=\`ps -ef | grep inetd | grep -v grep | awk '{print $2}'\`
echo Restarting pid \$pid
kill -HUP \$pid
GEN
chmod +x $dir/enable_clax_inetd_generic.sh

    cat >$dir/enable_clax_inetd_solaris.sh <<SOL
cp /etc/services $dir/services.bak
cp $dir/services.clax /etc/services
inetconv -f -i $dir/inetd.clax
inetadm -e svc:/network/clax/tcp:default
SOL
chmod +x $dir/enable_clax_inetd_solaris.sh

    cat >$dir/enable_clax_xinetd_generic.sh <<'XIN'
cp /etc/services $dir/services.bak
cp $dir/services.clax /etc/services
cp $dir/xinetd.clax >> /etc/xinetd.d/clax

pid=\`ps -ef | grep xinetd | grep -v grep | awk '{print $2}'\`
echo Restarting pid \$pid
kill -HUP \$pid
XIN
chmod +x $dir/enable_clax_xinetd_generic.sh

    cat <<EOC

To finish the inetd configuration do the following:

For inetd users in ALL *x platforms except SOLARIS:
  1.- Copy the $dir/services.clax file to /etc/services
  2.- Add the contents of $dir/inetd.clax to the /etc/inetd.conf file and restart inetd (ALL *x platforms except Solaris)

  or

  1.- Execute $dir/enable_clax_inetd_generic.sh (depending on the platform the command to restart inetd may vary. Please, check before execute)

For inetd users in SOLARIS:
  1.- Copy the $dir/services.clax file to /etc/services
  2.- Execute the following commands
     inetconv -f -i $dir/inetd.clax
     inetadm -e svc:/network/clax/tcp:default
  
  or
 
  1.- Execute $dir/enable_clax_inetd_solaris.sh

For xinetd users in all platforms:
  1.- Copy the $dir/services.clax file to /etc/services (ALL *x platforms)
  2.- Copy the file $dir/xinetd.clax to /etc/xinetd.d/clax and restart xinetd

  or

  1.- Execute $dir/enable_clax_xinetd_generic.sh (the command to restart xinetd may vary depending on the platform. Please, check before execute)
EOC
}

function usage {
  cat <<EOF

Usage: $progname [OPTIONS]

Options:
  -u username      The user that will own the clax software and will execute the service.  Defaults to current user
  -d path          The directory where clax will be installed. Defaults to /opt/clax
  -p port          The port where clax will be listening. Defaults to 11801
  -s               The installation will be silent.  No user interaction.
  -f               Force the installation     
EOF
}

function install_clax {
    echo "Installing clax in $dir"
    create_dir
    copy_binary
    create_ini
    generate_services
    generate_inetd
    change_owner
    echo "Installation finished in $dir"
    platform_config
}

if [ ! $username = "root" ]
then
   echo "This installation should be executed with the root user.  Otherwise some operations are likely to fail.  Do you want to continue (y/N)?"
    choice="y"
    read choice

    case $choice in 
      y|Y ) echo "Installing with $username user" ;;
      n|N ) echo "Canceling installation"; exit 1 ;;
      * ) echo "Invalid selection. Canceling installation"; exit 1 ;;
    esac
fi

while getopts ":u:d:p:shf" opt; do
   case $opt in

   u )  username=$OPTARG ;;
   d )  dir=$OPTARG ;;
   p )  port=$OPTARG ;;
   s )  silent="yes" ;;
   f )  force="yes" ;;
   h )  usage ; exit 0 ;;
   \?)  echo "\nInvalid option: -$OPTARG" >&2 ; usage ; exit 1 ;;
   esac
done

shift $(($OPTIND - 1))

cat <<EOF

Selected options:
 Owner=$username
 Install dir=$dir
 Port=$port
 Inetd=$inetd

EOF

if [ $silent = "no" ]
then
    echo "Are you sure you want to install clax with the selected options (Y/n)?"
    choice="y"
    read choice

    case $choice in 
      y|Y ) install_clax ;;
      n|N ) echo "Canceling installation"; exit 1 ;;
      * ) echo "Invalid selection. Canceling installation"; exit 1 ;;
    esac
else
    install_clax
fi
