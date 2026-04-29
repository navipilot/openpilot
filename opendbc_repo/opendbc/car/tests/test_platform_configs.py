from opendbc.car.chrysler.values import CAR as CHRYSLER_CAR, pacifica_hybrid_aol_stock_acc_mode
from opendbc.car.values import PLATFORMS


class TestPlatformConfigs:
  def test_configs(self, subtests):

    for name, platform in PLATFORMS.items():
      with subtests.test(platform=str(platform)):
        assert platform.config._frozen

        if platform != "MOCK":
          assert len(platform.config.dbc_dict) > 0
        assert len(platform.config.platform_str) > 0

        assert name == platform.config.platform_str

        assert platform.config.specs is not None

  def test_pacifica_hybrid_aol_stock_acc_mode_requires_pr_conditions(self):
    assert pacifica_hybrid_aol_stock_acc_mode(CHRYSLER_CAR.CHRYSLER_PACIFICA_2019_HYBRID, True, False, True)
    assert not pacifica_hybrid_aol_stock_acc_mode(CHRYSLER_CAR.CHRYSLER_PACIFICA_2019_HYBRID, True, True, True)
    assert not pacifica_hybrid_aol_stock_acc_mode(CHRYSLER_CAR.CHRYSLER_PACIFICA_2019_HYBRID, True, False, False)
    assert not pacifica_hybrid_aol_stock_acc_mode(CHRYSLER_CAR.CHRYSLER_PACIFICA_2019_HYBRID, False, False, True)

  def test_pacifica_hybrid_aol_stock_acc_mode_stays_narrow(self):
    assert not pacifica_hybrid_aol_stock_acc_mode(CHRYSLER_CAR.CHRYSLER_PACIFICA_2018_HYBRID, True, False, True)
    assert not pacifica_hybrid_aol_stock_acc_mode(CHRYSLER_CAR.CHRYSLER_PACIFICA_2020, True, False, True)
