# Building Ice for Ruby on Linux and macOS

This file describes how to build and install Ice for Ruby from source code on
Linux and macOS. If you prefer, you can also download [binary distributions][1]
for the supported platforms.

* [Ruby Build Requirements](#ruby-build-requirements)
  * [Operating Systems and Compilers](#operating-systems-and-compilers)
  * [Ruby Versions](#ruby-versions)
* [Building the Ruby Extension](#building-the-ruby-extension)
* [Installing Ice for Ruby](#installing-ice-for-ruby)
* [Configuring your Environment for Ruby](#configuring-your-environment-for-ruby)
* [Running the Ruby Tests](#running-the-ruby-tests)
* [SELinux Notes for Ruby](#selinux-notes-for-ruby)

## Ruby Build Requirements

### Operating Systems and Compilers

Ice for Ruby is expected to build and run properly on macOS and on any recent
Linux distribution for x86 and x86_64, and was extensively tested using the
operating systems and Ruby versions listed for our [supported platforms][2].

### Ruby Versions

Ice for Ruby supports Ruby versions 1.8.1 or later. You can use a source or
binary installation of Ruby.

If you use an RPM installation, the following packages are required:
```
ruby
ruby-devel
ruby-libs (RHEL)
```

## Building the Ruby Extension

The instructions for compiling the Ice extension assume that you have already
installed Ruby.

If you installed Ruby in a non-standard location, set the `RUBY_HOME`
environment variable to the installation directory. For example:
```
export RUBY_HOME=/opt/ruby
````

The build of Ice for Ruby requires that you first build Ice for C++ in the
`cpp` subdirectory.

From the top-level source directory, edit `config/Make.rules` to establish your
build configuration. The comments in the file provide more information.

Change to the Ice for Ruby source subdirectory:
```
cd ruby
```

Run `make` to build the extension.

## Installing Ice for Ruby

You can perform an automated installation with the following command:
```
make install
```

This process uses the `prefix` variable in `../config/Make.rules` as the
installation's root directory. The subdirectory `<prefix>/ruby` is created as a
copy of the local `ruby` directory and contains the Ice for Ruby extension
library as well as Ruby source code. Using this installation method requires
that you modify your environment as described in *Using Ice for Ruby* below.

Another option is to copy the contents of the local `ruby` directory to your
Ruby installation's `site_ruby` directory. For example, if you installed Ruby
via RPM, you can use the steps below:
```
cd <Ice source directory>/ruby/ruby
sudo tar cf - * | (cd /usr/lib/ruby/site_ruby/1.8/i386-linux; tar xvf -)
```

On x86_64 systems, change the last command to:
```
sudo tar cf - * | (cd /usr/lib64/ruby/site_ruby/1.8/x86_64-linux; tar xvf -)
```

There is no need to modify your environment if you use this approach.

## Configuring your Environment for Ruby

The Ruby interpreter must be able to locate the Ice extension. If you used the
automated installation described above, you need to define the `RUBYLIB`
environment variable as follows:
```
export RUBYLIB=/opt/Ice/ruby:$RUBYLIB
```

This example assumes that your Ice for Ruby installation is located in the
`/opt/Ice` directory.

You must also modify `LD_LIBRARY_PATH` or `DYLD_LIBRARY_PATH` to include the
directory `/opt/Ice/lib`:
```
export LD_LIBRARY_PATH=/opt/Ice/lib:$LD_LIBRARY_PATH       (Linux)
export DYLD_LIBRARY_PATH=/opt/Ice/lib:$DYLD_LIBRARY_PATH   (macOS)
```

To verify that Ruby can load the Ice extension successfully, open a command
window and start the interpreter using `irb`.

At the prompt, enter:
```
require "Ice"
```

If the interpreter responds with the value true, the Ice extension was loaded
successfully. Enter `exit` to quit the interpreter.

## Running the Ruby Tests

The `test` subdirectory contains Ruby implementations of the core Ice test
suite. Python is required to run the test suite.

The test suites require that the Ice for C++ tests be built in the `cpp`
subdirectory of this source distribution.

Open a command window and change to the top-level directory. At the command
prompt, execute:
```
python allTests.py
```

You can also run tests individually by changing to the test directory and
running this command:
```
python run.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## SELinux Notes for Ruby

If SELinux is enabled on your RHEL system, you may encounter this error message
when Ruby attempts to load the Ice extension:
```
cannot restore segment prot after reloc: Permission denied
```
There are two ways to solve this problem:

- Change the default security context for the Ice extension using the following
command:

    ```
    chcon -t texrel_shlib_t /opt/Ice/ruby/IceRuby.so
    ```

Replace `/opt/Ice` with your installation directory.

- Disable SELinux completely by adding the following line to your
`/etc/sysconfig/selinux` file:

    ```
    SELINUX=disabled
    ```

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.2
