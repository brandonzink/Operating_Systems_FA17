/*
NOTES: one of the requirements for this is that a flag is the user
namespace[1] be named pa4-encfs.encrypted that is set to either true
or false (if there is no flag, then assume it is not encrypted). I can't
find the implimentation of this anywhere.
*/

#define FUSE_USE_VERSION 28
#define HAVE_SETXATTR
#define ENCRYPT 1
#define DECRYPT 0
#define PASS -1
#define ENC_XATTR "pa4-encfs.encrypted"
#define DEBUG 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <limits.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "aes-crypt.h"

typedef struct {
    char *rootdir;
    char *key;
} encfs_state;


//Gets the filepath stored in fuse_context in private data, stores in the in state struct "state"
static void encfs_fullpath(char fpath[PATH_MAX], const char *path)
{
    if(DEBUG){printf("Entering fullpath.\n");}

    encfs_state *state = (encfs_state *) (fuse_get_context()->private_data);
    strcpy(fpath, state->rootdir);
    strncat(fpath, path, PATH_MAX);
}


//Return file attributes.
static int encfs_getattr(const char *path, struct stat *stbuf)
{
  if(DEBUG){printf("Entering getattr.\n");}

	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

  //returns info about the link, returns length of contents of the link
	res = lstat(fullpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}


//This is the same as the access(2) system call. It returns -ENOENT if the path doesn't
//exist, -EACCESS if the requested permission isn't available, or 0 for success.
static int encfs_access(const char *path, int amode)
{
  if(DEBUG){printf("Entering access.\n");}

	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = access(fullpath, amode);
	if (res == -1)
		return -errno;

	return 0;
}


// Places the contents of the symbolic link referred to by path in the
// buffer buf which has size bufsize
static int encfs_readlink(const char *path, char *buf, size_t size)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = readlink(fullpath, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


//Return one or more directory entries (struct dirent) to the caller
static int encfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

  char fullpath[PATH_MAX];

  encfs_fullpath(fullpath, path);

	(void) offset;
	(void) fi;

  //Opens the directory
	dp = opendir(fullpath);
	if (dp == NULL)
		return -errno;

  //while reading the directory
	while ((de = readdir(dp)) != NULL) {
    //FIGURE OUT WHAT THIS DOES, IT SEEMS IMPORTANT
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

  //close it back up
	closedir(dp);
	return 0;
}


//Make a special (device) file
static int encfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

  char fullpath[PATH_MAX];

  encfs_fullpath(fullpath, path);



/*
	// On Linux this could just be 'mknod(path, mode, rdev)' but this
	// is more portable
	if (S_ISREG(mode)) {
		res = open(fullpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(fullpath, mode);
	else
		res = mknod(fullpath, mode, rdev);
  */


  //create the file
  res = mknod(fullpath, mode, rdev);
  if (res == -1)
		return -errno;

	return 0;
}


//Create a directory with the given name. The directory permissions are encoded in mode.
static int encfs_mkdir(const char *path, mode_t mode)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = mkdir(fullpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}


//Remove (delete) the given file, symbolic link, hard link, or special node.
static int encfs_unlink(const char *path)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = unlink(fullpath);
	if (res == -1)
		return -errno;

	return 0;
}


//Remove the given directory. This should succeed only if the directory is empty.
static int encfs_rmdir(const char *path)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = rmdir(fullpath);
	if (res == -1)
		return -errno;

	return 0;
}


//Create a symbolic link named "from" which, when evaluated, will lead to "to".
static int encfs_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}


//Rename the file, directory, or other object "from" to the target "to".
static int encfs_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}


//Create a hard link between "from" and "to".
static int encfs_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}


//Change the mode (permissions) of the given object to the given new permissions.
static int encfs_chmod(const char *path, mode_t mode)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = chmod(fullpath, mode);
	if (res == -1)
		return -errno;

	return 0;
}

//Change the given object's owner and group to the provided values.
static int encfs_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = lchown(fullpath, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}


//Truncate or extend the given file so that it is precisely size bytes long.
static int encfs_truncate(const char *path, off_t size)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = truncate(fullpath, size);
	if (res == -1)
		return -errno;

	return 0;
}


//Update the last access time of the given object from ts[0] and the last
//modification time from ts[1].
static int encfs_utimens(const char *path, const struct timespec ts[2])
{
	int res;
	struct timeval tv[2];

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	tv[0].tv_sec = ts[0].tv_sec;
	tv[0].tv_usec = ts[0].tv_nsec / 1000;
	tv[1].tv_sec = ts[1].tv_sec;
	tv[1].tv_usec = ts[1].tv_nsec / 1000;

	res = utimes(fullpath, tv);
	if (res == -1)
		return -errno;

	return 0;
}


//Open a file.
static int encfs_open(const char *path, struct fuse_file_info *fi)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = open(fullpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

//Read size bytes from the given file into the buffer buf, beginning offset bytes into the file.
//FIGURE OUT HOW THIS ALL WORKS, CAUSE IT LOOKS LIKE FUCKING MAGIC
static int encfs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
    ssize_t valsize;
    char fullpath[PATH_MAX];
    FILE *fp, *tmp;

    encfs_fullpath(fullpath, path);

    (void) fi;

    valsize = getxattr(fullpath, ENC_XATTR, NULL, 0);
    if (valsize >= 0) {
        char* tmppath;
        tmppath = malloc(sizeof(char)*(strlen(fullpath) + strlen(".temp") + 1));
        tmppath[0] = '\0';
        strcat(tmppath, fullpath);
        strcat(tmppath, ".temp");

        tmp = fopen(tmppath, "wb+");
        fp = fopen(fullpath, "rb");

        do_crypt(fp, tmp, DECRYPT, ((encfs_state *) fuse_get_context()->private_data)->key);

        fseek(tmp, 0, SEEK_END);
        size_t len = ftell(tmp);
        fseek(tmp, 0, SEEK_SET);

        res = fread(buf, 1, len, tmp);
        if (res == -1)
            return -errno;

        fclose(fp);
        fclose(tmp);
        remove(tmppath);
    }
    else {
	    fd = open(fullpath, O_RDONLY);
	    if (fd == -1)
		    return -errno;

        res = pread(fd, buf, size, offset);
	    if (res == -1)
		    res = -errno;

    	close(fd);
    }

	return res;
}


//Write size bytes from the given file into the buffer buf, beginning offset bytes into the file.
//AGAIN, MAGIC I SAYS
static int encfs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
    ssize_t valsize;
    char fullpath[PATH_MAX];
    FILE *fp, *tmp;

    encfs_fullpath(fullpath, path);

    (void) fi;

    valsize = getxattr(fullpath, ENC_XATTR, NULL, 0);
    if (valsize >= 0) {
        char* tmppath;
        tmppath = malloc(sizeof(char)*(strlen(fullpath) + strlen(".temp") + 1));
        tmppath[0] = '\0';
        strcat(tmppath, fullpath);
        strcat(tmppath, ".temp");

        fp = fopen(fullpath, "rb+");
        tmp = fopen(tmppath, "wb+");

        fseek(fp, 0, SEEK_SET);

        do_crypt(fp, tmp, DECRYPT, ((encfs_state *) fuse_get_context()->private_data)->key);

        fseek(fp, 0, SEEK_SET);

        res = fwrite(buf, 1, size, tmp);
        if (res == -1)
            return -errno;

        fseek(tmp, 0, SEEK_SET);

        do_crypt(tmp, fp, ENCRYPT, ((encfs_state *) fuse_get_context()->private_data)->key);

        fclose(fp);
        fclose(tmp);
        remove(tmppath);
    }
    else {
	    fd = open(fullpath, O_WRONLY);
	    if (fd == -1)
		    return -errno;

        res = pwrite(fd, buf, size, offset);
	    if (res == -1)
		    res = -errno;

    	close(fd);
    }

	return res;
}

//Return statistics about the filesystem.
static int encfs_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	res = statvfs(fullpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

//NOT SURE WHAT THIS DOES YET, THERE'S NO DOCUMENTATION FOR IT ON THE MAIN PAGE
static int encfs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {

    (void) fi;

    FILE *fp;
    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

    int res;
    res = creat(fullpath, mode);
    if(res == -1)
	    return -errno;

    fp = fdopen(res, "w");
    close(res);

    do_crypt(fp, fp, ENCRYPT, ((encfs_state *) fuse_get_context()->private_data)->key);

    fclose(fp);

    if (setxattr(fullpath, ENC_XATTR, "true", 4, 0) == -1) {
        return -errno;
    }

    return 0;
}




/////////////////////////////////////////////////////////////////////////////////

//SOMETHING WITH SETXATTR? CHECK LATER
#ifdef HAVE_SETXATTR

//Set an extended attribute.
static int encfs_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	int res = lsetxattr(fullpath, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

//Read an extended attribute.
static int encfs_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	int res = lgetxattr(fullpath, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

//List the names of all extended attributes.
static int encfs_listxattr(const char *path, char *list, size_t size)
{
    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	int res = llistxattr(fullpath, list, size);
	if (res == -1)
		return -errno;
	return res;
}

//Remove an extended atribute.
static int encfs_removexattr(const char *path, const char *name)
{
    char fullpath[PATH_MAX];

    encfs_fullpath(fullpath, path);

	int res = lremovexattr(fullpath, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

/////////////////////////////////////////////////////////////////////////////////




//all of the function declaration usages
static struct fuse_operations encfs_oper = {
	.getattr	= encfs_getattr,
	.access		= encfs_access,
	.readlink	= encfs_readlink,
	.readdir	= encfs_readdir,
	.mknod		= encfs_mknod,
	.mkdir		= encfs_mkdir,
	.symlink	= encfs_symlink,
	.unlink		= encfs_unlink,
	.rmdir		= encfs_rmdir,
	.rename		= encfs_rename,
	.link		  = encfs_link,
	.chmod		= encfs_chmod,
	.chown		= encfs_chown,
	.truncate	= encfs_truncate,
	.utimens	= encfs_utimens,
	.open		  = encfs_open,
	.read		  = encfs_read,
	.write		= encfs_write,
	.statfs		= encfs_statfs,
	.create   = encfs_create,
	//.release	= encfs_release,
	//.fsync		= encfs_fsync,
#ifdef HAVE_SETXATTR
	.setxattr	= encfs_setxattr,
	.getxattr	= encfs_getxattr,
	.listxattr	= encfs_listxattr,
	.removexattr	= encfs_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	//any file permission may be used
	umask(0);

	//error check
    if (argc < 4) {
        fprintf(stderr, "Usage: %s %s \n", argv[0], "<Key> <Mirror Directory> <Mount Point>");
        return EXIT_FAILURE;
    }

    //initialize data
    encfs_state *encfs_data;

    //make the room
    encfs_data = malloc(sizeof(encfs_state));

    //set the encryption key and filepath directory
    encfs_data->key = argv[1];
    encfs_data->rootdir = realpath(argv[2], NULL);

    //start the execution

    //fuse_main() parses the arguments, and then calls fuse_mount() which creates a socket pair
    //and forks and execs fusermount

    //fuse_main() also calls fuse_new() which allocates the struct datastructure that stores and
    //caches and image of the filesystem data

    //fuse_main() also calls fuse_loop() which loops using the functions as defined above
	return fuse_main(argc - 2, argv + 2, &encfs_oper, encfs_data);
}
