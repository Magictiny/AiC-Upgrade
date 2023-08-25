use std::io::prelude::*;
use std::net::{TcpStream};
use ssh2::Session;
use std::env;

fn main() {
    // establish ssh connection
    let args: Vec<String> = env::args().collect();
    if args.len() < 7 { println!("Usage {} hostip username passwd 6.17.0 vcu 0.300.12980 ",&args[0]); return ;}
    let host = &args[1];
    let username = &args[2];
    let password = &args[3];
    let kuafu = &args[4];
    let vxu = &args[5];
    let new_version =  &args[6];

    let new_build_version = format!( "build_version={version}", version = &new_version );
    let cmd_edit_version = format!("cd kuafu-v{kuafu}/artifacts/ ; sed -i 's/^build.*/{version}/g' {vxu}_config.ini", version = &new_build_version , kuafu = &kuafu , vxu = &vxu );
    let cmd_to_workdir = format!("cd ; cd kuafu-v{kuafu}/scenarios/{vxu}*", kuafu = &kuafu , vxu = &vxu );
    let cmd_load_image = "./16*.sh";
    let cmd_update_rannic = "./17*.sh update";
    let cmd_delete_deploy = "./20*.sh";
    let cmd_create_deploy = "./21*.sh";
    let cmd_autoload_scf = "./42*.sh";
    let cmd_cp_scf_to_kuafu = format!(r"mv ~/xxx_scf_file.xml ~/kuafu-v{kuafu}/scf/{vxu}/ && echo 'scf process done' ",kuafu = &kuafu , vxu = &vxu );  //2023-0607,new version don't process scf, so add this.

    let cmd1 = format!("{}",&cmd_edit_version);
    println!("Start to edit version info...\ncmd is {}",&cmd_edit_version);
    exec_cmds(host,username,password,&cmd1);

    println!("Start to copy scf file to kuafu.");
    exec_cmds(host,username,password,&cmd_cp_scf_to_kuafu);                                   // copy scf file to kuafu.

    println!("Start to upgrade.....");
    let mut _cmd_upgrade;
    if vxu == "vdu" {
        _cmd_upgrade = format!("{};{};{};{};{};{}",cmd_to_workdir,cmd_load_image,cmd_update_rannic,cmd_delete_deploy,cmd_create_deploy,cmd_autoload_scf);
    } else {
        _cmd_upgrade = format!("{};{};{};{};{}",cmd_to_workdir,cmd_load_image,cmd_delete_deploy,cmd_create_deploy,cmd_autoload_scf);
    };
    
    println!( "cmd is: \n{}\nStart to upgrade..",&_cmd_upgrade);
    exec_cmds(host,username,password,&_cmd_upgrade);
    //println!("Start to try to remove images older than {}..., image node IP is:",&time_to_keep);
    //let _worker_ip = exec_cmds(host,username,password,&cmd2).trim().to_string();
    //exec_cmds(&_worker_ip ,username,password,&cmd3);
    //println!("Check image storage status after clearing...");
    //exec_cmds(host,username,password,&cmd1);
    println!("Upgraded, pls check result on you WebEM..");

}

fn exec_cmds( host: &String,username : &String, password : &String, cmds: &String) -> String
{
    let tcp = TcpStream::connect(format!("{}:22", host)).unwrap();
    let mut sess = Session::new().unwrap();
    sess.set_tcp_stream(tcp);
    sess.handshake().unwrap();
    sess.userauth_password(&username, &password).unwrap();
    let mut channel = sess.channel_session().unwrap();    // open a channel
    //channel.exec(r"image_pod=$(kubectl get pods -n openshift-image-registry | grep -w image-registry | grep -Ev cluster-image-registry-operator | awk '{print $1}');kubectl exec -it ${image_pod} -n openshift-image-registry -- df -h /registry").unwrap(); // execute command on the remote server
    channel.exec(&cmds).unwrap();
    // read output from the command
    let mut output = String::new();
    channel.read_to_string(&mut output).unwrap();
    println!("{}", &output);
    channel.send_eof().unwrap();
    channel.wait_close().unwrap();
    output
    // print the output
}

fn shell_cmds( host: &String,username : &String, password : &String, cmds: &String) //-> String
{
    let tcp = TcpStream::connect(format!("{}:22", host)).unwrap();
    let mut sess = Session::new().unwrap();
    sess.set_tcp_stream(tcp);
    sess.handshake().unwrap();
    sess.userauth_password(&username, &password).unwrap();

    let mut channel = sess.channel_session().unwrap();    // open a channel
    //channel.exec(r"image_pod=$(kubectl get pods -n openshift-image-registry | grep -w image-registry | grep -Ev cluster-image-registry-operator | awk '{print $1}');kubectl exec -it ${image_pod} -n openshift-image-registry -- df -h /registry").unwrap(); // execute command on the remote server
    let mut cmds = format!("{}\n",&cmds); 
    channel.write_all(b"&cmds").unwrap();
    //channel.exec(&cmds).unwrap();
    // read output from the command
    channel.send_eof().unwrap();
    channel.wait_close().unwrap();

    let mut output = String::new();
    channel.read_to_string(&mut output).unwrap();
    println!("{}", output);
    // print the output
}
