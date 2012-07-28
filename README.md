Description
-----------
    Study Unix networking programing.
    tags: C, socket, thread, libevent, lua, shell, php and so on.
    Create time: 2012.07

Notice
------
    libevent: libevent-2.0.19-stable, please install it at first.
    lua: lua-5.2.0
    php: need mbstring ext
    alias gw='gcc -g -O2 -Wall -fno-strict-aliasing -Wno-deprecated-declarations -D_THREAD_SAFE'
    alias gt='gcc -g -finline-functions -Wall -Winline -pipe'

The usage of simple suggestion system by prefix index
-----
    git clone git://github.com/hy0kl/unp.git
    cd unp/ev-sever
    vi auto.sh #change the work_path for your system.
    ./auto.sh

    visit: http://localhost:8080/?word=%E4%B8%80

History
------
    2012.08.27 finish one demo version.

Thanks
------
    Rick: yyr168@gmail.com

See Demo
--------
    demo/index.html
