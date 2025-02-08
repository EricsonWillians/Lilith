# Lilith Programming Language - Comprehensive Examples

## Table of Contents
1. [Basic Program Structure](#basic-program-structure)
2. [Variables and Assignments](#variables-and-assignments)
3. [Control Flow](#control-flow)
4. [Functions and Lambdas](#functions-and-lambdas)
5. [Collections and Comprehensions](#collections-and-comprehensions)
6. [Classes and Objects](#classes-and-objects)
7. [Pattern Matching](#pattern-matching)
8. [Exception Handling](#exception-handling)
9. [Parallel Processing](#parallel-processing)
10. [GPU Computing](#gpu-computing)
11. [Neural Network Operations](#neural-network-operations)
12. [Memory Management](#memory-management)
13. [Stream Processing](#stream-processing)
14. [Advanced Examples](#advanced-examples)

## Basic Program Structure

### Hello World
```lilith
{[
    @!(("Hello, World!"))
]}
```

### Basic Expressions
```lilith
{[
    /* Basic arithmetic */
    x [=] 5 ++ 3
    y [=] 10 -- 2
    z [=] x ** y

    /* String concatenation */
    name [=] "Alice" ++ " " ++ "Smith"
]}
```

## Variables and Assignments

### Basic Assignment
```lilith
{[
    number [=] 42
    text [=] "Hello"
    flag [=] #true
]}
```

### Multiple Assignments
```lilith
{[
    x [=] y [=] z [=] 0     /* Chain assignment */
    
    /* Parallel assignment via tuples */
    (<a,, b,, c>) [=] (<1,, 2,, 3>)
]}
```

## Control Flow

### If Statements
```lilith
{[
    [?((x ++ y == 10))
        [[
            @!(("Equal to 10"))
        ]]
        :|:
        [[
            @!(("Not equal to 10"))
        ]]
    ?]
]}
```

### Loops
```lilith
{[
    /* While loop */
    x [=] 0
    <+((x << 10))[[
        x [=] x ++ 1
        @!((x))
    ]]+>

    /* Loop with break */
    <+((#true))[[
        [?((x == 5))[[
            ]-!
        ]]?]
        x [=] x ++ 1
    ]]+>
]}
```

## Functions and Lambdas

### Basic Function
```lilith
{[
    (| add ((a,, b))
        [[
            )- a ++ b -(
        ]]
    |)

    result [=] add((5,, 3))
]}
```

### Function with Type Annotations
```lilith
{[
    (| calculate ((x (:) number,, y (:) number)) -> number
        [[
            )- x ** y -(
        ]]
    |)
]}
```

### Lambda Expression
```lilith
{[
    increment [=] (:<((x))[[x ++ 1]]>:)
    
    /* Using lambda in map */
    numbers [=] [<1,, 2,, 3>]
    mapped [=] map((numbers,, increment))
]}
```

### Async Function
```lilith
{[
    (| ~ fetch_data ((url))
        [[
            response [=] ~(http.get((url)))~
            )- response -(
        ]]
    |)
]}
```

## Collections and Comprehensions

### List Operations
```lilith
{[
    /* List literal */
    numbers [=] [<1,, 2,, 3,, 4,, 5>]
    
    /* List comprehension */
    squares [=] [<
        x ** 2
        [:< x [%] numbers >:]
        [?: x %% 2 == 0 :?]
    >]
]}
```

### Dictionary Operations
```lilith
{[
    /* Dictionary literal */
    person [=] {<
        "name" [:] "Alice",,
        "age" [:] 30,,
        "city" [:] "London"
    >}
    
    /* Dictionary comprehension */
    squared_map [=] {<
        x [:] x ** 2
        [:< x [%] [<1,, 2,, 3>] >:]
    >}
]}
```

### Set Operations
```lilith
{[
    /* Set literal */
    unique_numbers [=] [{1,, 2,, 2,, 3,, 3,, 4}]
    
    /* Set comprehension */
    even_squares [=] [{
        x ** 2
        [:< x [%] [<1,, 2,, 3,, 4,, 5>] >:]
        [?: x %% 2 == 0 :?]
    }]
]}
```

## Classes and Objects

### Basic Class
```lilith
{[
    {| Person
        [[
            (| init ((self,, name,, age))
                [[
                    self.name [=] name
                    self.age [=] age
                ]]
            |)

            (| greet ((self))
                [[
                    )- "Hello, " ++ self.name -(
                ]]
            |)
        ]]
    |}
]}
```

### Inheritance
```lilith
{[
    {| Animal
        [[
            (| speak ((self))
                [[
                    )- "..." -(
                ]]
            |)
        ]]
    |}

    {| Dog ([:Animal:])
        [[
            (| speak ((self))
                [[
                    )- "Woof!" -(
                ]]
            |)
        ]]
    |}
]}
```

## Pattern Matching

### Basic Pattern Matching
```lilith
{[
    (-< value >-)
        [<0>] [[
            @!(("Zero"))
        ]]
        [<1>] [[
            @!(("One"))
        ]]
        [<(<x,, y>)>] [[
            @!(("Tuple with " ++ x ++ " and " ++ y))
        ]]
]}
```

### Pattern Matching with Guards
```lilith
{[
    (-< number >-)
        [<x>] [?: x << 0 :?] [[
            @!(("Negative"))
        ]]
        [<x>] [?: x == 0 :?] [[
            @!(("Zero"))
        ]]
        [<x>] [[
            @!(("Positive"))
        ]]
]}
```

## Exception Handling

### Basic Try-Except
```lilith
{[
    {?
        [[
            result [=] dangerous_operation(())
        ]]
        [! error [/]
            [[
                @!(("Error: " ++ error))
            ]]
        !]
    ?}
]}
```

### Try-Except-Finally
```lilith
{[
    {?
        [[
            file [=] open(("data.txt"))
        ]]
        [! error [/]
            [[
                @!(("Error: " ++ error))
            ]]
        !]
        [:~
            [[
                file.close(())
            ]]
        ~:]
    ?}
]}
```

## Parallel Processing

### Basic Parallel Loop
```lilith
{[
    <|((4))[[  /* Parallel with 4 threads */
        heavy_computation(())
    ]]|>
]}
```

### Parallel with Synchronization
```lilith
{[
    <|((threads [:] 8,, sync [:] barrier))[[
        compute_partial_results(())
        barrier.wait(())
        combine_results(())
    ]]|>
]}
```

## GPU Computing

### Basic GPU Kernel
```lilith
{[
    <%((
        grid [:] [<256,, 1,, 1>],,
        block [:] [<256,, 1,, 1>]
    ))[[
        i [=] blockIdx.x ** blockDim.x ++ threadIdx.x
        result[i] [=] data[i] ** 2
    ]]%>
]}
```

### Matrix Multiplication on GPU
```lilith
{[
    /* Define the GPU kernel */
    (| ~ matrix_mul_kernel ((A,, B,, C))
        [[
            <%((
                grid [:] [<16,, 16,, 1>],,
                block [:] [<16,, 16,, 1>]
            ))[[
                row [=] blockIdx.y ** blockDim.y ++ threadIdx.y
                col [=] blockIdx.x ** blockDim.x ++ threadIdx.x
                sum [=] 0
                
                <+((k << A.width))[[
                    sum [=] sum ++ A[row][k] ** B[k][col]
                ]]+>
                
                C[row][col] [=] sum
            ]]%>
        ]]
    |)
]}
```

## Neural Network Operations

### Define Neural Network Layer
```lilith
{[
    <%%((layer [:] linear,, units [:] 128))[[
        W [=] init_weights((784,, 128))
        b [=] init_bias((128))
        
        (| forward ((x))[[
            )- activate((W ** x ++ b)) -(
        ]]|)
    ]]%%>
]}
```

### Convolution Layer
```lilith
{[
    <%%((
        layer [:] conv2d,,
        filters [:] 64,,
        kernel [:] [<3,, 3>]
    ))[[
        (| forward ((x))[[
            )- conv2d_forward((x,, weights,, bias)) -(
        ]]|)
    ]]%%>
]}
```

## Memory Management

### Explicit Memory Layout
```lilith
{[
    [^((type [:] static,, align [:] 256))
        [[
            matrix [=] allocate((1024,, 1024))
        ]]
    ^]
]}
```

### GPU Memory Management
```lilith
{[
    [^((type [:] gpu,, memory [:] pinned))
        [[
            data [=] allocate_gpu((size [:] 1024))
            copy_to_gpu((source [:] host_data,, dest [:] data))
        ]]
    ^]
]}
```

## Stream Processing

### Basic Stream Processing
```lilith
{[
    <~((source [:] data_stream,, window [:] 100))
        [[
            value [=] process_window((current_window))
            emit((value))
        ]]
    ~>
]}
```

### Complex Stream Processing
```lilith
{[
    <~((
        source [:] sensor_data,,
        window [:] [<size [:] 1000,, slide [:] 100>]
    ))[[
        /* Process each window */
        avg [=] compute_average((current_window))
        
        [?((avg >> threshold))[[
            emit_alert((avg))
        ]]?]
    ]]~>
]}
```

## Advanced Examples

### Neural Network Training Loop
```lilith
{[
    (| train ((model,, data,, epochs))[[
        <+((epoch << epochs))[[
            <~((batch [%] data))[[
                /* Forward pass */
                predictions [=] model.forward((batch.x))
                loss [=] compute_loss((predictions,, batch.y))
                
                /* Backward pass */
                gradients [=] compute_gradients((loss))
                
                /* Update on GPU */
                <%((threads [:] 256))[[
                    update_weights((model.parameters,, gradients))
                ]]%>
            ]]~>
        ]]+>
    ]]|)
]}
```

### Parallel Data Processing Pipeline
```lilith
{[
    /* Define the pipeline */
    (| process_data ((input_stream))[[
        <~((source [:] input_stream,, batch [:] 1000))[[
            /* Stage 1: Preprocess on CPU */
            preprocessed [=] preprocess((current_batch))
            
            /* Stage 2: GPU computation */
            <%((grid [:] [<16,, 16>]))[[
                result [=] compute_on_gpu((preprocessed))
            ]]%>
            
            /* Stage 3: Postprocess results */
            <|((threads [:] 4))[[
                postprocess((result))
            ]]|>
            
            /* Stage 4: Store results */
            store_results((result))
        ]]~>
    ]]|)
]}