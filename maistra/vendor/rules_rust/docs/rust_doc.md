<!-- Generated with Stardoc: http://skydoc.bazel.build -->
# Rust Doc

* [rust_doc](#rust_doc)
* [rust_doc_test](#rust_doc_test)

<a id="#rust_doc"></a>

## rust_doc

<pre>
rust_doc(<a href="#rust_doc-name">name</a>, <a href="#rust_doc-dep">dep</a>, <a href="#rust_doc-html_after_content">html_after_content</a>, <a href="#rust_doc-html_before_content">html_before_content</a>, <a href="#rust_doc-html_in_header">html_in_header</a>, <a href="#rust_doc-markdown_css">markdown_css</a>)
</pre>

Generates code documentation.

Example:
  Suppose you have the following directory structure for a Rust library crate:

  ```
  [workspace]/
      WORKSPACE
      hello_lib/
          BUILD
          src/
              lib.rs
  ```

  To build [`rustdoc`][rustdoc] documentation for the `hello_lib` crate, define   a `rust_doc` rule that depends on the the `hello_lib` `rust_library` target:

  [rustdoc]: https://doc.rust-lang.org/book/documentation.html

  ```python
  package(default_visibility = ["//visibility:public"])

  load("@rules_rust//rust:rust.bzl", "rust_library", "rust_doc")

  rust_library(
      name = "hello_lib",
      srcs = ["src/lib.rs"],
  )

  rust_doc(
      name = "hello_lib_doc",
      dep = ":hello_lib",
  )
  ```

  Running `bazel build //hello_lib:hello_lib_doc` will build a zip file containing   the documentation for the `hello_lib` library crate generated by `rustdoc`.


**ATTRIBUTES**


| Name  | Description | Type | Mandatory | Default |
| :------------- | :------------- | :------------- | :------------- | :------------- |
| <a id="rust_doc-name"></a>name |  A unique name for this target.   | <a href="https://bazel.build/docs/build-ref.html#name">Name</a> | required |  |
| <a id="rust_doc-dep"></a>dep |  The label of the target to generate code documentation for.<br><br><code>rust_doc</code> can generate HTML code documentation for the source files of <code>rust_library</code> or <code>rust_binary</code> targets.   | <a href="https://bazel.build/docs/build-ref.html#labels">Label</a> | required |  |
| <a id="rust_doc-html_after_content"></a>html_after_content |  File to add in <code>&lt;body&gt;</code>, after content.   | <a href="https://bazel.build/docs/build-ref.html#labels">Label</a> | optional | None |
| <a id="rust_doc-html_before_content"></a>html_before_content |  File to add in <code>&lt;body&gt;</code>, before content.   | <a href="https://bazel.build/docs/build-ref.html#labels">Label</a> | optional | None |
| <a id="rust_doc-html_in_header"></a>html_in_header |  File to add to <code>&lt;head&gt;</code>.   | <a href="https://bazel.build/docs/build-ref.html#labels">Label</a> | optional | None |
| <a id="rust_doc-markdown_css"></a>markdown_css |  CSS files to include via <code>&lt;link&gt;</code> in a rendered Markdown file.   | <a href="https://bazel.build/docs/build-ref.html#labels">List of labels</a> | optional | [] |


<a id="#rust_doc_test"></a>

## rust_doc_test

<pre>
rust_doc_test(<a href="#rust_doc_test-name">name</a>, <a href="#rust_doc_test-dep">dep</a>)
</pre>

Runs Rust documentation tests.

Example:

Suppose you have the following directory structure for a Rust library crate:

```output
[workspace]/
  WORKSPACE
  hello_lib/
      BUILD
      src/
          lib.rs
```

To run [documentation tests][doc-test] for the `hello_lib` crate, define a `rust_doc_test` target that depends on the `hello_lib` `rust_library` target:

[doc-test]: https://doc.rust-lang.org/book/documentation.html#documentation-as-tests

```python
package(default_visibility = ["//visibility:public"])

load("@rules_rust//rust:rust.bzl", "rust_library", "rust_doc_test")

rust_library(
    name = "hello_lib",
    srcs = ["src/lib.rs"],
)

rust_doc_test(
    name = "hello_lib_doc_test",
    dep = ":hello_lib",
)
```

Running `bazel test //hello_lib:hello_lib_doc_test` will run all documentation tests for the `hello_lib` library crate.


**ATTRIBUTES**


| Name  | Description | Type | Mandatory | Default |
| :------------- | :------------- | :------------- | :------------- | :------------- |
| <a id="rust_doc_test-name"></a>name |  A unique name for this target.   | <a href="https://bazel.build/docs/build-ref.html#name">Name</a> | required |  |
| <a id="rust_doc_test-dep"></a>dep |  The label of the target to run documentation tests for.<br><br><code>rust_doc_test</code> can run documentation tests for the source files of <code>rust_library</code> or <code>rust_binary</code> targets.   | <a href="https://bazel.build/docs/build-ref.html#labels">Label</a> | required |  |

