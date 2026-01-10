
## Electronics

#### Quiescent Current
| Item | Current draw in shutdown | Condition |
|:--:|:--:|:--:|
| ATtiny202 | 6u | Standby |
| LM2759 LED Driver | 9.7u | Shutdown |
| APW7261 Battery Charger | 55u | unplugged |
| TOTAL | 70.7uA |  |

##### Battery life
$$
\begin{align}
\frac{Battery Capacity}{Current Consumption} =& Hours \\
\frac{Battery Capacity}{0.0000707 A} =& Hours \\
=& Hours
\end{align}
$$