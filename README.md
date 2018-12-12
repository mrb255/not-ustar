# not-ustar

An implementation of a contiguously allocated filesystem using the FUSE (Filesystem in User SpacE) library. 

**Compile:**

```gcc -g -O0 -Wall myfs.c implementation.c `pkg-config fuse --cflags --libs` -o myfs```

**Mount:**

```./myfs --backupfile=test.myfs ~/fuse-mnt/ -f```

**Unmount:**

```fusermount -u ~/fuse-mnt```

**Install FUSE:**

```sudo apt-get install libfuse-dev```

This project was completed per the requirements of the CSCE A321 - Operating Systems course at the University of Alaska Anchorage.
