/// <reference path="libs/js/stream-deck.js" />
/// <reference path="key-register.js" />
/// <reference path="debug-action-manager.js" />
/// <reference path="debug-action.js" />

const playInEditorAction = new DebugAction('com.augkit.unreal-editor.playInEditor', 'stoped');
playInEditorAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('play')
})
const stopInEditorAction = new DebugAction('com.augkit.unreal-editor.stopInEditor', 'running');
stopInEditorAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('stop')
})

$SD.onConnected(({ actionInfo, appInfo, connection, messageType, port, uuid }) => {
	$DAM.switchStep('linking')
})

$DAM.start('55346')

