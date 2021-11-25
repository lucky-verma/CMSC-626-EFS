## **Getting Started**:


### Setup the environment:

#### Install GoLang:
```
$ sudo apt update
$ sudo apt upgrade

$ sudo apt install libssl-dev gcc pkg-config

$ sudo apt search golang-go
$ sudo apt search gccgo-go
```

```
$ sudo apt install golang-go
```

### Install gocryptfs:

```
$ git clone https://github.com/lucky-verma/CMSC-626-EFS.git
$ cd CMSC-626-EFS/gocryptfs
$ ./build-without-openssl.bash
```

### Commands for EFS:

#### Create:

```
$ mkdir cipher plain

$ gocryptfs -init cipher
```


#### Mount:

```
$ gocryptfs cipher plain
```


#### Unmount:

```
$ fusermount -u “folder name”
```


#### Change Password:

```
$ gocryptfs -passwd [OPTIONS] “cipherDir”
```


## **INSTALL GUI**:

```
$ echo 'deb http://download.opensuse.org/repositories/home:/obs_mhogomchungu/xUbuntu_20.04/ /' | sudo tee /etc/apt/sources.list.d/home:obs_mhogomchungu.list
$ curl -fsSL https://download.opensuse.org/repositories/home:obs_mhogomchungu/xUbuntu_20.04/Release.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/home_obs_mhogomchungu.gpg > /dev/null
$ sudo apt update
$ sudo apt install sirikali
```

#### Running GUI

```
$ sirikali
```
