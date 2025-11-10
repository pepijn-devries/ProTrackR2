# Select and assign operators for ProTrackR2 S3 class objects

Functions to select and assign elements to ProTracker modules. See
[`vignette('s3class')`](https://pepijn-devries.github.io/ProTrackR2/articles/s3class.md)
for an overview of ProTrackR2 S3 class objects. See
[`vignette('sel_assign')`](https://pepijn-devries.github.io/ProTrackR2/articles/sel_assign.md)
for practical guidance on selecting and assigning elements of ProTrackR2
class objects.

## Usage

``` r
# S3 method for class 'pt2mod'
`$`(x, i, ...)

# S3 method for class 'pt2mod'
x$i <- value

# S3 method for class 'pt2mod'
x[[i]] <- value

# S3 method for class 'pt2mod'
x[[i, ...]]

# S3 method for class 'pt2patlist'
x[i, ...]

# S3 method for class 'pt2patlist'
x[[i, ...]]

# S3 method for class 'pt2patlist'
x[[i]] <- value

# S3 method for class 'pt2samplist'
x[i, ...]

# S3 method for class 'pt2samplist'
x[[i, ...]]

# S3 method for class 'pt2samplist'
x[[i]] <- value

# S3 method for class 'pt2pat'
x[[i, ...]]

# S3 method for class 'pt2pat'
x[[i]] <- value

# S3 method for class 'pt2pat'
x[i, j, ...]

# S3 method for class 'pt2pat'
x[i, j, ...] <- value

# S3 method for class 'pt2celllist'
x[[i, ...]] <- value

# S3 method for class 'pt2celllist'
x[i, ...] <- value

# S3 method for class 'pt2celllist'
x[[i, ...]]

# S3 method for class 'pt2celllist'
x[i, ...]

# S3 method for class 'pt2command'
x[[i, ...]]

# S3 method for class 'pt2command'
x[i, ...]

# S3 method for class 'pt2command'
x[[i, ...]] <- value

# S3 method for class 'pt2command'
x[i, ...] <- value
```

## Arguments

- x:

  Object to apply S3 method to. See 'usage' section for allowed object
  types.

- i, j:

  Indices for extracting or replacing ProTrackR2 object elements

- ...:

  Passed on to other methods.

- value:

  Replacement value for the selected element(s).

## Value

Returns the selected object in case of a selection (`[`, `[[`, or `$`)
operator. Returns the updated object `x` in case of an assignment (`<-`)
operator.
