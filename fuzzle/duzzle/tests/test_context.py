from fuzzle.duzzle.archs import x86_64
from fuzzle.duzzle.core.context import DuzzleContext


def test_init():
    """
    Test DuzzleContext initialisation.
    """

    # Create DuzzleContext
    duzzle = DuzzleContext(arch=x86_64)

    # Check init
    resp = duzzle.init()
    assert(resp['type'] == 'result')
    assert(resp['message'] == 'done')
    assert(resp['payload'] == None)
    assert(resp['token'] == 0)
    assert(resp['stream'] == 'stdout')

def test_connect():

    

if __name__ == '__main__':
    """
    Main entry point.
    """

    # Execute tests
    test_init()
    test_connect()