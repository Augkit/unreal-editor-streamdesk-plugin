/// <reference path="libs/js/stream-deck.js" />
/// <reference path="key-register.js" />
/// <reference path="debug-action-manager.js" />
/// <reference path="debug-action.js" />

const playInEditorAction = new DebugAction('com.augkit.unreal-editor.playInEditor', 'stoped', 'play');
playInEditorAction.onKeyUp(({ action, context, device, event, payload }) => {
	if ($KR.getKeyContetxtState(context) === 'resume') {
		$DAM.doAction('resume')
	} else {
		$DAM.doAction('play')
	}
})
const stopInEditorAction = new DebugAction('com.augkit.unreal-editor.stopInEditor', 'running');
stopInEditorAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('stop')
})


const debugLocateAction = new DebugAction('com.augkit.unreal-editor.debugLocate', 'running');
debugLocateAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('debug_locate')
})
const debugAbortAction = new DebugAction('com.augkit.unreal-editor.debugAbort', 'running');
debugAbortAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('debug_abort')
})
const debugContinueAction = new DebugAction('com.augkit.unreal-editor.debugContinue', 'running');
debugContinueAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('debug_continue')
})
const debugStepIntoAction = new DebugAction('com.augkit.unreal-editor.debugStepInto', 'running');
debugStepIntoAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('debug_step_into')
})
const debugStepOverAction = new DebugAction('com.augkit.unreal-editor.debugStepOver', 'running');
debugStepOverAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('debug_step_over')
})
const debugStepOutAction = new DebugAction('com.augkit.unreal-editor.debugStepOut', 'running');
debugStepOutAction.onKeyUp(({ action, context, device, event, payload }) => {
	$DAM.doAction('debug_step_out')
})

let websocket
$SD.onConnected(({ actionInfo, appInfo, connection, messageType, port, uuid }) => {
	$DAM.switchStep('linking')
	$DAM.start('35346')

	// websocket = new WebSocket("ws://127.0.0.1:" + "11111")

	// websocket.onopen = () => {
	// }

	// websocket.onerror = (evt) => {
	// 	websocket.close()
	// }
})
