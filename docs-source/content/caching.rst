Caching
=======

For better compatibility with the bs::framework, BsZenLib can *cache* the original
games files in the format bs::f expects.
Since bs::f does not support using the actual Importer-modules with content comming
from a package format, BsZenLib has to craft the bs::f resources manually. After doing
so, it will save the imported resources into the ``cache``-directory.

Example
-------

To generate the cache for a texture, you would use the functions 
described in the chapter :ref:`import-textures`.
You will also need to set up a virtual file index containing the texture you
want to import. See :ref:`vdfs` on how to populate one.

Suppose you would want to import the texture ``STONE.TGA``:

.. code-block:: c

    #include <BsZenLib/ImportTexture.hpp>

    // ...

    HTexture ImportStoneTga()
    {
        VDFS::FileIndex vdfs = ...;

        return BsZenLib::ImportAndCacheTexture("STONE.TGA", vdfs);
    }

This will search for the compiled version of that texture (``STONE-C.TEX``), load it
and write it to the cache.