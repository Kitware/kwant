Kitware Analytics Toolkit
=========================

KWANT is an open source C++ toolkit for computing scores and other metrics for object tracking systems.

Pending support for e.g. GeographicLib in kwiver, this release only supports pixel-coordinate AOIs.

For more information on how KWANT achieves this goal,
and how to use KWANT visit our `documentation site <http://kwant.readthedocs.io/en/latest/>`_

Directory Structure and Provided Functionality
==============================================

======================= ===========================================================
`<CMake>`_              CMake helper scripts
`<doc>`_                Documentation, manuals, release notes
`<scoring_framework>`_  The scoring algorithms
`<utilities>`_          Various utility filters
======================= ===========================================================

Building KWANT
===============

Dependencies
------------
KWANT requires, at a minimum, Git, CMake, and a C++ compiler.

KWANT is built on top of the `KWIVER <https://github.com/Kitware/kwiver>`_ toolkit.
which is in turn built on the `Fletch <https://github.com/Kitware/fletch>`_ super build system.

You will need to have KWIVER already built for KWANT to use when building.


Running CMake
-------------

You may run cmake directly from a shell or cmd window.
On unix systems, the ccmake tool allows for interactive selection of CMake options.  
Available for all platforms, the CMake GUI can set the source and build directories, options,
"Configure" and "Generate" the build files all with the click of a few button.

We recommend building KWANT out of its source directory to prevent mixing
source files with compiled products.  Create a build directory in parallel
with the KWANT source directory for each desired configuration. For example :

========================== ===================================================================
``\KWANT\src``               contains the code from the git repository
``\KWANT\build\release``     contains the built files for the release configuration
``\KWANT\build\debug``       contains the built files for the debug configuration
========================== ===================================================================

Basic CMake generation via command line
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following example will pull and build Fletch and KWIVER along with the DIVA code base.
It assumes your terminal/command is working in the ``\DIVA\build\release`` directory. ::

    # cmake usage : $ cmake </path/to/kwiver/source> -D<flags>
    $ cmake ../../src -DCMAKE_BUILD_TYPE=Release 

Using a prebuilt Fletch
~~~~~~~~~~~~~~~~~~~~~~~

If you would like to point DIVA to a prebuilt version of Fletch, specify the fletch_DIR flag to cmake.
The fletch_DIR is the fletch build directory root, which contains the fletchConfig.cmake file. ::

    $ cmake ../../src -DCMAKE_BUILD_TYPE=Release -Dfletch_DIR:PATH=<path/to/fletch/build/dir> 

You must ensure that the specified build of Fletch has enabled all the appropriate flags for use by KWIVER and DIVA.
The required flags can be found in this file : `<CMake/add_project_fletch.cmake>`_ 

Using a prebuilt KWIVER
~~~~~~~~~~~~~~~~~~~~~~~

If you would like to point DIVA to a prebuilt version of KWIVER, specify the kwiver_DIR flag to cmake.
The kwiver_DIR is the KWIVER build directory root, which contains the kwiver-config.cmake file. 
*NOTE* As KWIVER requires a Fletch directory, the build will ignore the fletch_DIR variable and use the Fletch that was used to build KWIVER. ::

    $ cmake ../../src -DCMAKE_BUILD_TYPE=Release -Dkwiver_DIR:PATH=<path/to/kwiver/build/dir> 

You must ensure that the specified build of KWIVER was build with a fletch that was built with all necessary options.
KWIVER must have also been built with all the appropriate flags for use by DIVA.
The required flags can be found in this file : `<CMake/add_project_kwiver.cmake>`_ 

This framework requires `track_oracle` to be turned on in [kwiver](https://github.com/Kitware/kwiver).

Compiling
---------

Once your CMake generation has completed and created the build files,
compile in the standard way for your build environment.  On Linux
this is typically running ``make``. Visual Studio users, open the <path/to/KWANT/build/dir>/KWANT.sln

Getting Help
============

Please join the
`kwiver-users <http://public.kitware.com/mailman/listinfo/kwiver-users>`_
mailing list to discuss DIVA/KWIVER or to ask for help with using DIVA/KWIVER.
For announcements about DIVA and other projects built on KWIVER, please join the
`kwiver-announce <http://public.kitware.com/mailman/listinfo/kwiver-announce>`_
mailing list.
