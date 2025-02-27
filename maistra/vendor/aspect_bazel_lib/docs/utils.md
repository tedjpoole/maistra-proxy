<!-- Generated with Stardoc: http://skydoc.bazel.build -->

Public API

<a id="default_timeout"></a>

## default_timeout

<pre>
default_timeout(<a href="#default_timeout-size">size</a>, <a href="#default_timeout-timeout">timeout</a>)
</pre>

Provide a sane default for *_test timeout attribute.

The [test-encyclopedia](https://bazel.build/reference/test-encyclopedia) says
> Tests may return arbitrarily fast regardless of timeout.
> A test is not penalized for an overgenerous timeout, although a warning may be issued:
> you should generally set your timeout as tight as you can without incurring any flakiness.

However Bazel's default for timeout is medium, which is dumb given this guidance.

It also says:
> Tests which do not explicitly specify a timeout have one implied based on the test's size as follows
Therefore if size is specified, we should allow timeout to take its implied default.
If neither is set, then we can fix Bazel's wrong default here to avoid warnings under
`--test_verbose_timeout_warnings`.

This function can be used in a macro which wraps a testing rule.


**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="default_timeout-size"></a>size |  the size attribute of a test target   |  none |
| <a id="default_timeout-timeout"></a>timeout |  the timeout attribute of a test target   |  none |

**RETURNS**

"short" if neither is set, otherwise timeout


<a id="file_exists"></a>

## file_exists

<pre>
file_exists(<a href="#file_exists-path">path</a>)
</pre>

Check whether a file exists.

Useful in macros to set defaults for a configuration file if it is present.
This can only be called during the loading phase, not from a rule implementation.


**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="file_exists-path"></a>path |  a label, or a string which is a path relative to this package   |  none |


<a id="glob_directories"></a>

## glob_directories

<pre>
glob_directories(<a href="#glob_directories-include">include</a>, <a href="#glob_directories-kwargs">kwargs</a>)
</pre>



**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="glob_directories-include"></a>include |  <p align="center"> - </p>   |  none |
| <a id="glob_directories-kwargs"></a>kwargs |  <p align="center"> - </p>   |  none |


<a id="is_external_label"></a>

## is_external_label

<pre>
is_external_label(<a href="#is_external_label-param">param</a>)
</pre>

Returns True if the given Label (or stringy version of a label) represents a target outside of the workspace

**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="is_external_label-param"></a>param |  a string or label   |  none |

**RETURNS**

a bool


<a id="path_to_workspace_root"></a>

## path_to_workspace_root

<pre>
path_to_workspace_root()
</pre>

 Returns the path to the workspace root under bazel


**RETURNS**

Path to the workspace root


<a id="propagate_well_known_tags"></a>

## propagate_well_known_tags

<pre>
propagate_well_known_tags(<a href="#propagate_well_known_tags-tags">tags</a>)
</pre>

Returns a list of tags filtered from the input set that only contains the ones that are considered "well known"

These are listed in Bazel's documentation:
https://docs.bazel.build/versions/main/test-encyclopedia.html#tag-conventions
https://docs.bazel.build/versions/main/be/common-definitions.html#common-attributes


**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="propagate_well_known_tags-tags"></a>tags |  List of tags to filter   |  <code>[]</code> |

**RETURNS**

List of tags that only contains the well known set


<a id="to_label"></a>

## to_label

<pre>
to_label(<a href="#to_label-param">param</a>)
</pre>

Converts a string to a Label. If Label is supplied, the same label is returned.

**PARAMETERS**


| Name  | Description | Default Value |
| :------------- | :------------- | :------------- |
| <a id="to_label-param"></a>param |  a string representing a label or a Label   |  none |

**RETURNS**

a Label


