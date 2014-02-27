# Repository guidelines
The libfluid project is formed by three repositories:

* **libfluid_base**: a library that provides the connection layer for the 
OpenFlow protocol
* **libfluid_msg**: a library that provides the messaging layer for the 
OpenFlow protocol
* **libfluid**: a bundle that aggregates **libfluid_base** and 
**libfluid_msg**, providing extra documentation, examples and useful scripts 
for building.

# libfluid version system
**libluid_base** and **libfluid_msg** are the core libraries. Their releases 
will be based on a annotated tag based on a commit. The version policy will 
follow the [Semantic Versioning specification](http://semver.org/). The first 
version of libfluid is `v0.1.0`.

The **libfluid** repository itself does not feature versions. It points to 
minor releases of both **libfluid_base** and **libfluid_msg**. Its content must 
always be stable in the master branch and it must always work with the releases 
of **libfluid_base** and **libfluid_msg** it is using.

When the **libfluid** repository is bootstraped, a patch version will be chosen 
for a the minor version it's configured to use (e.g. if it's declaring 0.1, and 
there's a 0.1.5 release, then the `v0.1.5` tag will be chosen). If 
`bootstrap.sh` is run with the `dev` flag (as in `./bootstrap.sh dev`), the 
`master` branch will be used instead of a stable release (useful for 
development).

# What should I use?
If you just want to get started with libfluid, try the **libfluid** repository.

If you want to use only **libfluid_base** or **libfluid_msg** (they don't 
depend on each other), you can clone their repositories directly. You can also 
choose any tagged release in these repositories.

# How should developers work?
Developers working on libfluid should clone **libfluid_base** and 
**libfluid_msg** directly and work on that. If you use the **libfluid** 
repository, make sure you bootstrap the repository with the `dev` flag so that
the `master` branch is kept as the selected branch:
    
~~~{.sh}
$ ./bootstrap.sh dev
~~~

Features must be developed in branches and be merged into the `master` branch 
only when they are ready for a release.

External developers can submit pull requests in a branch. They will be reviewed
by the libfluid team before accepting them, and eventually merged into the 
`master` branch.

Simple patch (bug fix) commits can be commited directly on the `master` branch. 
Other changes should be developed in separate branches.

When the `master` branch is considered to be good enough, a new version will be 
tagged.
