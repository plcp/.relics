
.. role:: sh(code)
   :language: sh

==================================
First steps with :literal:`fuddly`
==================================

Today, we will start a quick fuzzing_ session with fuddly_, a *fuzzing and data
manipulation framework (for GNU/Linux)*, as a follow-up to the tutorial_
provided by fuddly's documentation_.

.. _fuddly:        https://github.com/k0retux/fuddly
.. _fuzzing:       https://en.wikipedia.org/wiki/Fuzzing
.. _tutorial:      http://fuddly.readthedocs.io/en/develop/tutorial/
.. _documentation: http://fuddly.readthedocs.io/en/develop/

Quick setup
-----------

We are going to work within the :literal:`develop` branch of
fuddly's repository (here_).

.. _here: https://github.com/plcp/fuddly/tree/develop

**This write-up should be up-to-date with the mainline version of fuddly**,
but if you want to replay the commands given with the exact same setup than the
one used at the time of writing, you may clone the repository and the
write-up's :literal:`Makefile` will use fuddly's `commit 1c87a98`_:

.. _`commit 1c87a98`: https://github.com/k0retux/fuddly/commit/1c87a98180ddf11b00b21f07fcbfb37f372d3a75

.. code:: sh

    $ git clone --recursive https://github.com/plcp/fuddly-wu
    $ cd 1
    $ make setup # checkout the commit used at the time of writing

You will find a :literal:`Makefile` bundled with some helpers in our working
directory.

.. code:: sh

    $ cd fuddly-wu/1
    $ make list

Note that we specify :sh:`cd fuddly-wu/1` to implicitly indicate that we
are in the working directory.


Starting up the :literal:`fuddly shell`
---------------------------------------

You can interact with the framework via a regular `python interpreter`_ or via
a `custom interpreter`_. The custom interpreter provide an easier access to
high-level primitives, a native completion for fuddly-specific keywords, and
above all, a better look.

.. _`python interpreter`: http://fuddly.readthedocs.io/en/develop/tutorial/#using-fuddly-through-advanced-python-interpreter
.. _`custom interpreter`: http://fuddly.readthedocs.io/en/develop/tutorial/#using-fuddly-simple-ui-fuddly-shell

(with python)
^^^^^^^^^^^^^

You can start a shell by executing :literal:`fuddly/fuddly_shell.py` with
python:

.. code:: sh

    $ cd fuddly-wu/1
    $ make setup # specific to our write-ups
    $ python ./fuddly/fuddly_shell.py

(with the write-up's helper)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can also use the provided helpers to setup the write-up's environment and to
start a proper fuddly shell:

.. code:: sh

    $ cd fuddly-wu/1
    $ make setup shell

Note that these :literal:`Makefiles` that we provide are not a requirement of
fuddly, these are just a convenience as part of this series of write-ups.

Troubleshooting
^^^^^^^^^^^^^^^

The fuddly shell should be colorfull, if not, it should report some missing
dependencies. The :literal:`Makefile` should setup a :literal:`virtualenv` with
the required dependencies, thus you may try to remove it:

.. code:: sh

    $ cd fuddly-wu/1
    $ rm -rf fuddly/venv ../venv
    $ make

Note that the helper is using :literal:`python3` (even if fuddly is compatible
with :literal:`python2.7` and :literal:`python3`) and :literal:`virtualenv`,
thus you may need to install the dependencies by yourself if you are still
using :literal:`python2` or don't want to install :literal:`virtualenv`.

Fuzzing :literal:`unzip` with :literal:`fuzzy`
----------------------------------------------

First, start by giving yourself a decent shell:

.. code:: sh

    $ cd fuddly-wu/1
    $ make

We will now follow fuddly's documentation tutorial_ inside the fuddly shell.

Loading a project
^^^^^^^^^^^^^^^^^

First, **we load the provided project** :literal:`standard` to fuzz our beloved
:literal:`unzip`:

.. code::

    -=[ Fuddly Shell ]=- (with Fuddly FmK 0.25.2)
    
    >> show_projects
    
    -=[ Projects ]=-
    
    [0] standard
    [1] usb
    
    >> load_project standard
    
    >> 

In a nutshell, **a project is just a bundle of configuration tweaks**, tools and
snippets of python code. For now, we are not going to detail what is or how
to create a project.

Selecting :literal:`unzip` as target
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Fuddly enables the fuzzing of mutliple targets. One target represent either
a program, a remote server, a system, or any kind of machinery that can be
fuzzed in some ways. We can list available targets as follows:

.. code::

    >> show_targets

    -=[ Available Targets ]=-
    
    [0] EmptyTarget
    [1] LocalTarget [Program: /usr/bin/display]
         \-- monitored by: display_mem_check(refresh=0.10s)
    [2] LocalTarget [Program: okular]
    [3] LocalTarget [Program: unzip, Args: -d /tmp/fuddly-wu/1/fuddly_data/workspace/]
    [4] PrinterTarget [IP: 127.0.0.1, Name: PDF]
    [5] NetworkTarget [localhost:54321#STREAM (serv:True,hold:False), localhost:12345#STREAM (serv:False,hold:True)]
    [6] NetworkTarget [localhost:12345#STREAM (serv:True,hold:True)]
    [7] NetworkTarget [eth0:3#RAW (serv:False,hold:True)]

    >>


**We select the target attached to** :literal:`unzip`, the third one in the
list:

.. code::
    
    >> set_target 3
    
    >>
    
Note that we will see how to define our own targets (and how to write the
*glue* between fuddly and our programs) in a next chapter (*krm*, write-up),
as what is the minimal target that we can write or what is the concept hidden
behind.

That said, we have now selected the target defined for :literal:`unzip` and now
we may want to send payloads of corrupted data to make the whole thing blowing
up.

If you want more details about things you may want to do now, see:
 - `passing arguments to unzip`_,
 - or `editing the unzip's target`_.

Crafting fine payloads with :literal:`fuddly`
---------------------------------------------

Fuddly provides a mean to represent any kind of payloads (from a file format to
a specific frame of an obscure protocol) with the so-called data models, but as
for the targets, we will see later what it is in the details.

**For now, we will just load the** :literal:`zip` **data model** and it will
provide us plenty of payloads to corrupt to feed our target:

.. code::
    
    >> show_data_models
    
    -=[ Data Models ]=-
    
    [0] example
    [1] mydf
    [2] pdf
    [3] zip
    [4] jpg
    [5] png
    [6] HTTP
    [7] usb
    [8] pppoe
    [9] sms
    
    >> load_data_model zip

We have now the key pieces of our evil scheme (fuzzing :literal:`unzip`) in our
hands as we have selected a project, a target and a data model. We may now
want to ask fuddly to put everything together, as it's time to break things.

**Thus, we have to** :literal:`launch` **the fuzzing session**:

.. code::

    >> launch
    *** Logger is started ***
    *** Data Model 'zip' loaded ***
    *** Target initialization ***
    *** Monitor is started ***
    
    *** [ Fuzz delay = 0.0s ] ***
    *** [ Number of data sent in burst = 1 ] ***
    *** [ Target health-check timeout = 10.0s ] ***
    >> 

And then, send a finely crafted payload to our target:

.. code::

    >> send ZIP

    ====[ 1 ]==[ 23/03/2017 - 17:12:17 ]====...
    ### Step 1:
     |- generator type: ZIP | generator name: g_zip | User input: G=[ ], S=[ ]
    ### Data size: 104 bytes
    ### Data emitted:
    b'PK\x03\x04<... payload here ...>&\x00\x00\x00'
    
    ### FmkDB Data ID: 1
    
    ### Target ack received at: None
    ### Target Feedback from 'LocalTarget[stderr]' (status=-2):
    Application outputs on stderr
    ### Target Feedback (status=-1):
    Archive:  /tmp/fuddly-wu/1/fuddly_data/workspace/fuzz_test_002434003688.zip
    
    
      End-of-central-directory signature not found.  Either this file is not
      a zipfile, or it constitutes <... rest of the output ...>

**We successfully send a payload to** :literal:`unzip`, but the one crafted
from the data model was discarded as an unvalid zip file. It's not a bad new
as we might expect from :literal:`unzip` to reject ill-formed zip files, but
we notice that the fuzzing session will be more interesting if we provide
*valid-but-still-a-bit-ill-formed* zip files.

Fortunately enough, fuddly provides us the mean to start from existing payloads
(and above all valid ones), leveraging the abstractions and the inner
constraints provided by the data model, and to produce slightly-corrupted
variants around one peculiar valid payload.

If you want more details about things you may want to do now, see:
 - `fuzzing unzip with thousands of payloads`_,
 - or `retrieving past payloads send for analysis`_.


Start your fuzzing from an existing payload
-------------------------------------------

We have now seen to load a project, choose a target, pick what kind of payloads
to send to our target, putting everything together, and sending ill-formed
payloads to our target.

We now want to use valid zip files to produce corrupted payloads (which
will hopefully cover a wider range of test cases). We're going to use
`droste.zip`_ (provided here: :literal:`fuddly-wu/1/droste.zip`) as an
example, but you may pick any zip files of your library.

In order to import the zip files into fuddly, we first need to copy them into
:literal:`fuddly_data/imported_data/zip/` and then :literal:`reload_all`.

.. code:: sh

    $ cd fuddly-wu/1
    $ make mr_proper setup # cleanup
    $ mkdir -p fuddly_data/imported_data/zip/
    $ cp *.zip fuddly_data/imported_data/zip/ # copy everything
    $ make shell

If you have closed your fuddly shell, do not forget to reload your project, your
target and your data model (here :literal:`unzip`-related):

.. code::

    >> load_project standard
    
    >> set_target 3
    
    >> load_data_model zip
    
    >> launch
    *** Logger is started ***
    ZIP_00 Absorb Status: <AbsorbStatus.FullyAbsorbed: 4>, 0, 754, ZIP_00
     \_ length of original zip: 754
     \_ remaining: b''
    --> Create ZIP_00 from provided ZIP samples.
    ZIP_01 Absorb Status: <AbsorbStatus.FullyAbsorbed: 4>, 0, 28809, ZIP_01
     \_ length of original zip: 28809
     \_ remaining: b''
    --> Create ZIP_01 from provided ZIP samples.
    *** Data Model 'zip' loaded ***
    *** Target initialization ***
    *** Monitor is started ***
    
    *** [ Fuzz delay = 0.0s ] ***
    *** [ Number of data sent in burst = 1 ] ***
    *** [ Target health-check timeout = 10.0s ] ***
    >>

Note the slightly different output now:

.. code::

    ZIP_00 Absorb Status: <AbsorbStatus.FullyAbsorbed: 4>, 0, 754, ZIP_00
     \_ length of original zip: 754
     \_ remaining: b''
    --> Create ZIP_00 from provided ZIP samples.
    ZIP_01 Absorb Status: <AbsorbStatus.FullyAbsorbed: 4>, 0, 28809, ZIP_01
     \_ length of original zip: 28809
     \_ remaining: b''
    --> Create ZIP_01 from provided ZIP samples.
    *** Data Model 'zip' loaded ***

The data model have *absorbed* the provided zip files. Now, we can send these
zip files as payloads:

.. code::

    >> comment
    (with the project, the target, the data model loaded and *launched*)

    >> send 
    
    


.. _`droste.zip`: https://alf.nu/ZipQuine

Further details
---------------

This write-up was pretty straigh-forward, thus you will find here some details
about the things we intentionnaly skipped, but which might interest an attentive
reader.

Passing arguments to :literal:`unzip`
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Previously, fuddly told us some details about :literal:`unzip`'s target:
 
.. code::

    >> show_targets

    -=[ Available Targets ]=-
    
    [0] EmptyTarget
    ...
    [3] LocalTarget [Program: unzip, Args: -d /tmp/fuddly-wu/1/fuddly_data/workspace/]
    ...


Fuddly will pass the specified arguments to :literal:`unzip`, hence
:literal:`unzip` will extract the corrupted archives inside
:literal:`/tmp/fuddly-wu/1/fuddly_data/workspace/`.

You have to edit :literal:`unzip`'s target to modify these arguments.

You may now want to:
 - see `editing the unzip's target`_,
 - or continue by `crafting fine payloads with fuddly`_.

Editing the :literal:`unzip`'s target
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**If you want to edit your target**, remember that it is defined inside your
project's file. As we're using the :literal:`standard` project, we can expect
to find :literal:`unzip`'s target defined inside the project file:

.. code:: sh

    $ cd fuddly-wu/1
    $ grep -rni "unzip" fuddly/projects | grep target
    fuddly/projects/generic/standard_proj.py:56:local3_tg.set_target_path('unzip')

We can find the target's definition by taking a look inside
:literal:`standard_proj.py` (added comments):

.. code:: python
   
    # [...]
    
    # commit 1c87a98180ddf11b00b21f07fcbfb37f372d3a75
    # l.55   /fuddly/projects/generic/standard_proj.py
    local3_tg = LocalTarget(tmpfile_ext='.zip')
    local3_tg.set_target_path('unzip')
    local3_tg.set_post_args('-d ' + gr.workspace_folder)
    
    # [...]
    
    # commit 1c87a98180ddf11b00b21f07fcbfb37f372d3a75
    # l.72   /fuddly/projects/generic/standard_proj.py
    targets = [(local_tg, # [...]
               local2_tg,
               local3_tg, # Note our target listed here
               # [...]

     # [...]

We might modify our target by editing our project file and tweaking its
configuration. For example, we have silently patched the commit during the
write-up's setup to add unzip's :literal:`-o` option before [#before]_ the
file name inserted during the fuzzing session:

.. code:: diff

    diff --git a/projects/generic/standard_proj.py b/projects/generic/standard_proj.py
    index 3f59e36..f5172b7 100644
    --- a/projects/generic/standard_proj.py
    +++ b/projects/generic/standard_proj.py
    @@ -54,7 +54,7 @@ local2_tg.set_target_path('okular')
    
    local3_tg = LocalTarget(tmpfile_ext='.zip')
    local3_tg.set_target_path('unzip')
    +local3_tg.set_pre_args('-o ')
    local3_tg.set_post_args('-d ' + gr.workspace_folder)
    
.. [#before]

    As described here__, subsection :literal:`Usage example`,
    item :literal:`line 5`, about `LocalTarget.set_pre_args()`__.

__ http://fuddly.readthedocs.io/en/develop/targets/#localtarget
__ http://fuddly.readthedocs.io/en/develop/framework/#framework.targets.local.LocalTarget.set_pre_args

You shouldn't have to modify the project file nor the data model during this
write-up (we're going to create our own project files later), but if you modify
the :literal:`standard`'s project file nevertheless, please **do not forget to
reload everything** in the fuddly shell:

.. code::

    >> load_project standard
    
    >> set_target 3
    
    >> load_data_model zip
    
    >> launch

    >> comment
    (do some things here, send generated zip files, other things...)

    >> comment
    !! Here, we modify our project file, a target, a data model.
    
    >> comment
    !! We need now to reload the components we have modified.

    >> reload_all
    
    >> 
    
(It's :code:`reload_all` that you want to notice here)

The key thing to remember here is that **the typical workflow** with fuddly
involves keeping the fuddly shell open somewhere for high-level manipulation of
the framework and editing your python files on the side to handle the details. 

You may now want to continue your reading by `crafting fine payloads with
fuddly`_.

Fuzzing :literal:`unzip` with *thousands* of payloads
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You may wonder how you are supposed to *actually fuzz something* by sending
payloads one at a time with :code:`send ZIP`. You don't, you just use
:code:`send_loop <n> ZIP` instead (note that it may take a while):

.. code::

    >> send_loop 16 ZIP

    >>

As an insight of `the next subsection`__ would have tell us, the :literal:`ZIP`
in :code:`send ZIP` refers to what we call a *generator*. Typing one time
:code:`send ZIP` ask the generator for the next payload "*scheduled*" for the
fuzzing, typing :code:`send_loop 16 ZIP` send the next 16 ones that the
generator can provide.

__ `Start your fuzzing from an existing payload`_

Typing :code:`send_loop -1 ZIP` will enumerate each possible payload that the
generator could produce (a lot), testing for each combination of edgy test
cases produced by the abstraction and the constraints of the data model.

.. code::

    >> send_loop -1 ZIP

    >>

You may now want to:
 - see `retrieving past payloads send for analysis`_,
 - or continue by `start your fuzzing from an existing payload`_.

Retrieving past payloads send for analysis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As you may have tried, sending `thousands of payloads`__ will generate a lot of
backlog to put through a detailed analysis by hand. By chanche, the default
setup of fuddly will store every bit of the payloads send and related
informations into a :literal:`SQLite` database located
in :literal:`fuddly_data/fmkdb.db`.

__ `Fuzzing unzip with thousands of payloads`_

An helper tool is provided to easily interact with this database in
:literal:`./fuddly/tools/fmkdb.py` (and we have replaced it by a wrapper to
handle :literal:`virtalenv`-related issues transparently and to keep our work
contained in the write-up's directory).

You may query for the 12th data send to our target by issuing the following
command:

.. code:: sh

    $ ./fuddly/tools/fmkdb.py -i 12

We can also retrieve the data and the feedback issued by :literal:`unzip`:

.. code:: sh

    $ ./fuddly/tools/fmkdb.py -i 12 --with-data --with-fbk

We do recommand to read the `attached section`__ of fuddly's documentation
tutorial.

__ http://fuddly.readthedocs.io/en/develop/tutorial/#how-to-send-a-zip-file

You may now want to continue your reading by
`start your fuzzing from an existing payload`_.
