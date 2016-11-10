module.exports = [
	{ 
    	"type": "heading", 
    	"defaultValue": "Settings",
		  "size": 1
	},
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"defaultValue": "Color Selection"
			},
			{
				"type": "toggle",
				"messageKey": "KEY_INVERT_COLORS",
				"label": "Invert Colors",
				"defaultValue": false
			},      
			{
				"type": "toggle",
				"messageKey": "KEY_INVERT_HAND_COLORS",
				"label": "Invert Hand Colors",
				"defaultValue": false
			},   
		]
	},
	{
		"type": "submit",
		"defaultValue": "Apply"
	},
	{
		"type": "text",
		"defaultValue": " Version 1.0"
	}
];

