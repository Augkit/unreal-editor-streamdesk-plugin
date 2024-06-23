/// <reference path="libs/js/action.js" />
/// <reference path="debug-action-manager.js" />
/// <reference path="action-register.js" />

class DebugAction extends Action {
    enableState = ''
    constructor(UUID, enableState = '') {
        super(UUID)
        this.enableState = enableState
        this.onWillAppear(({ action, context, device, event, payload }) => {
            $AR.register(context, this)
            if ($DAM.state == 'running') {
                $SD.setState(context, 0)
            } else {
                $SD.setState(context, 1)
            }
        });
        this.onWillDisappear(({ action, context, device, event, payload }) => {
            $AR.unregister(context, this)
        });
    }
}