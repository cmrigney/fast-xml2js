{
  "targets": [
    {
      "target_name": "fast-xml2js",
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "sources": [ "fast-xml2js.cpp", "rapidxml/rapidxml.hpp", "rapidxml/rapidxml_iterators.hpp", "rapidxml/rapidxml_print.hpp", "rapidxml/rapidxml_utils.hpp" ],
      "cflags!": ['-fno-exceptions'],
      "cflags_cc!": ['-fno-exceptions'],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "fast-xml2js" ],
      "copies": [
          {
            "files": [ "<(PRODUCT_DIR)/fast-xml2js.node" ],
            "destination": "fast-xml2js"
          }
      ]
    }
  ]
}
