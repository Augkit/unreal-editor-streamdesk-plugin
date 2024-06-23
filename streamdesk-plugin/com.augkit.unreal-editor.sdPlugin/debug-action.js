/// <reference path="libs/js/action.js" />
/// <reference path="debug-action-manager.js" />
/// <reference path="key-register.js" />

class DebugAction extends Action {
    enableStep = ''
    specialKey = ''
    constructor(UUID, enableStep = '', specialKey = '') {
        super(UUID)
        this.enableStep = enableStep
        this.specialKey = specialKey
        this.onWillAppear(({ action, context, device, event, payload }) => {
            $KR.register(context, this)
        })
        this.onWillDisappear(({ action, context, device, event, payload }) => {
            $KR.unregister(context, this)
        })
    }
}