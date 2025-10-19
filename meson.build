project('my-cpp-project', 'cpp',
    version: '1.0.0',
    default_options: [
        'cpp_std=c++20',
        'warning_level=3',
        'werror=true', # Treat all warnings as errors
        'optimization=s' # Optimize for size
    ]
)

cpp_args = [
    # Recommended warnings for g++/clang
    '-Wpedantic',
    '-Wformat=2',
    '-Wundef',
    '-Wcast-align',
    '-Wwrite-strings',
    '-Wold-style-cast',
    '-Wnull-dereference',
    '-Wctor-dtor-privacy',
    '-Wno-missing-field-initializers',
]

add_global_arguments(cpp_args, language : 'cpp')

app_sources = [
    'main.cpp',
    
    # User Classes
    'library/User/user.cpp',
    'library/User/buyer.cpp',
    'library/User/seller.cpp',
    
    # Item & Order Classes
    'library/Item/item.cpp',
    'library/Item/order.cpp',
    'library/Item/analytics.cpp',
    
    # Banking Classes
    'library/Bank/bank_customer.cpp',
    'library/Bank/bank.cpp',
    
    # Serialization Logic
    'library/Serialization/serialization.cpp',
]

executable('system-transaction',
    app_sources,
    # Example dependencies:
    # dependencies: [fmt_dep],
    install: true
)