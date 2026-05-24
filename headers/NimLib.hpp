struct Exec {
    const char* output;
    long        exit_code;
};

struct Fileinfo {
    const char* name;
    const char* creator;
    long        last_edit;
    long        file_size;
};

struct Folderinfo {
    const char* name;
    const char* creator;
    long        last_edit;
    long        folder_size;
};

struct Getargs {
    const char** args;
    long         length;
};

struct Getermsize {
    long columns;
    long rows;
};

struct Ladefs {
    const char** name;
    const char** value;
    long         length;
};

struct Laprocs {
    const char** name;
    long*        process_id;
    long         length;
};

struct Lf {
    const char** paths;
    const char** type_of;
    long         length;
};

struct Procinfo {
    const char* name;
    long        process_id;
    long        parent_process_id;
    const char* user_name;
    long        start_time;
    const char* command;
};

extern "C" {
    void*       allocbuf   (long size);
    void        cf         (const char* path);
    void        clrscr     (void);
    void        cron       (long milliseconds, void (*function)(void));
    void        cursor     (long visible);
    void        cursorto   (long x, long y);
    void        def        (const char* name, const char* value);
    Exec        exec       (const char* command);
    Fileinfo    fileinfo   (const char* path);
    const char* fileread   (const char* path);
    void        filewrite  (const char* path, const char* content);
    Folderinfo  folderinfo (const char* path);
    void        freebuf    (void* pointer1);
    Getargs     getargs    (void);
    const char* getcf      (void);
    const char* getchr     (void);
    long        getcpid    (void);
    const char* getdef     (const char* name);
    const char* getprogloc (void);
    Getermsize  getermsize (void);
    void        halt       (long exit_code);
    long        has        (const char* text, const char* pattern);
    long        isdef      (const char* name);
    long        isfile     (const char* path);
    long        isfolder   (const char* path);
    long        isroot     (void);
    void        killjob    (void (*function)(void));
    long        killproc   (long process_id);
    void        killthr    (void (*function)(void));
    Ladefs      ladefs     (void);
    Laprocs     laprocs    (void);
    Lf          lf         (void);
    void        mcopy      (const char* source, void* destination, long size);
    void        mkfile     (const char* path);
    void        mkfolder   (const char* path);
    void        mvfile     (const char* old_path, const char* new_path);
    void        mvfolder   (const char* old_path, const char* new_path);
    long        ping       (const char* url);
    Procinfo    procinfo   (long process_id);
    long        randint    (long minimum, long maximum);
    void*       reallocbuf (void* pointer1, long new_size);
    void        resetbgfg  (void);
    void        rmfile     (const char* path);
    void        rmfolder   (const char* path);
    void        schedule   (long milliseconds, void (*function)(void));
    void        scope      (long atexit, void (*function)(void));
    void        setbg      (long r, long g, long b);
    void        setfg      (long r, long g, long b);
    void        sig        (long signal, void (*function)(void));
    void        spawnthr   (void (*function)(void));
    long        spawnproc  (const char* command);
    const char* stdi       (long visible);
    void        stdo       (const char* string1);
    long        strcomp    (const char* string1, const char* string2);
    const char* strformat  (long count, ...);
    long        strsize    (const char* string1);
    void        syncthr    (void (*function)(void));
    double      timern     (void);
    void        timerstart (const char* name);
    long        timerstop  (const char* name);
    const char* tob64str   (long forurl, const char* value, const char* key);
    double      tofltint   (long value);
    double      tofltstr   (const char* value);
    long        tointflt   (double value);
    long        tointstr   (const char* value);
    const char* tostrb64   (const char* value, const char* key);
    const char* tostrflt   (double value);
    const char* tostrint   (long value);
    void        undef      (const char* name);
    void        until      (double timestamp);
    const char* version    (void);
    void        vidbuf     (long mode);
    void        wait       (long milliseconds);
    const char* where      (const char* command);
}
