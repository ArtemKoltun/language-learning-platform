# Математическая модель системы управления процессом забывания

## Аннотация

Документ описывает математическую модель адаптивной системы интервального повторения. Система сочетает модель памяти (feedforward), ПИД-регулятор (feedback) и расширенный фильтр Калмана для онлайн-оценки параметров. Модель предназначена для замены классических алгоритмов SM-2 и FSRS.

---

## 1. Модель памяти

### 1.1. Кривая забывания

Кривая забывания описывается степенной функцией с аддитивным сдвигом:

$$
R(t) = a + b \cdot t^{-c}, \quad t > 0
$$

где:

- $a$ — асимптота (уровень «вечной» памяти), $a \in [0, 1)$;
- $b$ — начальный уровень запоминания, $b > 0$;
- $c$ — скорость забывания, $c > 0$;
- $t$ — время с последнего повторения (в днях).

Ограничение: $R(t) \in [0, 1]$.

### 1.2. Смысл параметров

| Параметр | Роль | Влияние на кривую |
|---|---|---|
| $a$ | Асимптота | Устанавливает «пол» памяти |
| $b$ | Амплитуда | Сдвигает кривую по вертикали |
| $c$ | Показатель формы | Управляет темпом падения (наклон в log-log) |

**Нормализованная форма.** Вынесем $b$ за скобки:

$$
\tilde{R}(t) = \frac{R(t) - a}{b} = t^{-c}
$$

Величина $\tilde{R}(t)$ не зависит от $b$ — только от $c$. Это «чистая форма» кривой.

**Log-log представление.**

$$
\ln \tilde{R}(t) = -c \cdot \ln t
$$

Прямая линия в координатах $(\ln t, \ln \tilde{R})$ с наклоном $-c$. Чем больше $c$, тем круче наклон — тем быстрее забывание.

---

## 2. Feedforward: предсказание интервала

Находим $t$, при котором $R(t) = R_{\text{target}}$:

$$
a + b \cdot t^{-c} = R_{\text{target}}
$$

Решаем относительно $t$:

$$
t^{-c} = \frac{R_{\text{target}} - a}{b}
$$

$$
I_{\text{pred}} = \left( \frac{R_{\text{target}} - a}{b} \right)^{-1/c}
$$

При $a = 0$ упрощается до классической степенной:

$$
I_{\text{pred}} = \left( \frac{R_{\text{target}}}{b} \right)^{-1/c}
$$

Ограничение:

$$
I_{\text{pred}} = \text{clamp}\left( I_{\text{pred}},\ I_{\min},\ I_{\max} \right)
$$

---

## 3. ПИД-регулятор

### 3.1. Ошибка

$$
e(t) = R_{\text{target}} - \text{score}(t)
$$

где $\text{score} \in [0, 1]$ — непрерывная оценка ответа (получается через каскад exact → fuzzy → LLM).

### 3.2. Пропорциональная составляющая

$$
u_P = K_p \cdot e
$$

Реагирует на текущую ошибку. Роль во времени: **настоящее**.

### 3.3. Интегральная составляющая

$$
E_{\text{int}} \leftarrow E_{\text{int}} + e
$$

$$
E_{\text{int}} = \text{clamp}\left( E_{\text{int}},\ -E_{\max},\ E_{\max} \right)
$$

$$
u_I = K_i \cdot E_{\text{int}}
$$

Компенсирует накопленную ошибку. Роль во времени: **прошлое**.

**Anti-windup:** если $I_{\text{new}}$ упирается в границу, $E_{\text{int}}$ не накапливается.

**Сброс:** при $\text{score} < \theta_{\text{lapse}}$ → $E_{\text{int}} = 0$.

### 3.4. Дифференциальная составляющая

$$
\Delta e_{\text{raw}} = e - e_{\text{prev}}
$$

$$
\Delta e = \gamma \cdot \Delta e_{\text{raw}} + (1 - \gamma) \cdot \Delta e_{\text{prev}}
$$

$$
u_D = K_d \cdot \Delta e
$$

где $\gamma \in (0, 1]$ — коэффициент сглаживания.

Гасит колебания, реагирует на тренд. Роль во времени: **будущее**.

### 3.5. Выход ПИД

$$
u = K_p \cdot e + K_i \cdot E_{\text{int}} + K_d \cdot \Delta e
$$

---

## 4. Итоговый интервал

$$
I_{\text{new}} = I_{\text{pred}} \cdot (1 + u)
$$

Ограничения:

$$
I_{\text{new}} = \text{clamp}\left( I_{\text{new}},\ 0.5 \cdot I_{\text{old}},\ 2.0 \cdot I_{\text{old}} \right)
$$

$$
I_{\text{new}} = \text{clamp}\left( I_{\text{new}},\ I_{\min},\ I_{\max} \right)
$$

---

## 5. Расширенный фильтр Калмана

Поскольку модель памяти нелинейна по параметрам $a, b, c$, используется **расширенный фильтр Калмана (EKF)**, который линеаризует модель наблюдения в окрестности текущей оценки через матрицу Якоби.

### 5.1. Вектор состояния

$$
\mathbf{x} = \begin{bmatrix} a \\ b \\ c \end{bmatrix}
$$

### 5.2. Модель наблюдения

Наблюдаем $R(t_i)$ с шумом:

$$
z_i = R(t_i) + v_i = a + b \cdot t_i^{-c} + v_i, \quad v_i \sim \mathcal{N}(0, \sigma_v^2)
$$

### 5.3. Шаг предсказания

$$
\hat{\mathbf{x}}_{k|k-1} = \hat{\mathbf{x}}_{k-1|k-1}
$$

$$
\mathbf{P}_{k|k-1} = \mathbf{P}_{k-1|k-1} + \mathbf{Q}
$$

где $\mathbf{Q}$ — ковариация шума процесса.

### 5.4. Шаг коррекции

**Якобиан модели наблюдения:**

$$
\mathbf{H}_k = \begin{bmatrix} \dfrac{\partial h}{\partial a} & \dfrac{\partial h}{\partial b} & \dfrac{\partial h}{\partial c} \end{bmatrix}
= \begin{bmatrix} 1 & t_k^{-c} & -b \cdot t_k^{-c} \cdot \ln t_k \end{bmatrix}
$$

**Усиление Калмана:**

$$
\mathbf{K}_k = \mathbf{P}_{k|k-1} \mathbf{H}_k^\top \left( \mathbf{H}_k \mathbf{P}_{k|k-1} \mathbf{H}_k^\top + \sigma_v^2 \right)^{-1}
$$

**Обновление состояния:**

$$
\hat{\mathbf{x}}_{k|k} = \hat{\mathbf{x}}_{k|k-1} + \mathbf{K}_k \left( z_k - h(\hat{\mathbf{x}}_{k|k-1}) \right)
$$

**Обновление ковариации:**

$$
\mathbf{P}_{k|k} = \left( \mathbf{I} - \mathbf{K}_k \mathbf{H}_k \right) \mathbf{P}_{k|k-1}
$$

где $h(\mathbf{x}) = a + b \cdot t^{-c}$ — модель наблюдения.

### 5.5. Идентифицируемость

Для надёжной оценки трёх параметров требуется серия наблюдений $R(t_1), R(t_2), \ldots$ в разные моменты времени. Дефолтные параметры $a_0, b_0, c_0$ служат априорным распределением с широкой ковариацией $\mathbf{P}_0$, которое стабилизирует EKF на старте. По мере накопления данных $\mathbf{P}$ сжимается.

---

## 6. Обновление прочности памяти

### 6.1. Нормализация времени ответа

$$
\tau_{\text{norm}} = \frac{t_{\text{response}}}{\max(\text{len}(\text{answer}), 1)}
$$

### 6.2. Фактор скорости

$$
f_{\text{speed}} = \exp\left( -\frac{\tau_{\text{norm}}}{T_0} \right)
$$

### 6.3. Фактор насыщения

$$
f_{\text{sat}} = \frac{1}{1 + m / m_{\text{sat}}}
$$

### 6.4. Приращение при успехе

$$
\Delta m_+ = \eta_+ \cdot \text{score} \cdot f_{\text{speed}} \cdot f_{\text{sat}}
$$

### 6.5. Приращение при провале

$$
\Delta m_- = -\eta_- \cdot (1 - \text{score}) \cdot \left( 1 + \frac{m}{m_{\text{ref}}} \right)
$$

### 6.6. Итоговое обновление

$$
m_{\text{new}} = \text{clamp}\left( m + \text{score} \cdot \Delta m_+ + (1 - \text{score}) \cdot \Delta m_-,\ 0,\ m_{\max} \right)
$$

---

## 7. Каскадная схема

$$
\underbrace{\text{EKF}}_{\text{оценка } a, b, c}
\;\longrightarrow\;
\underbrace{R(t) = a + b t^{-c}}_{\text{модель памяти}}
\;\longrightarrow\;
\underbrace{I_{\text{pred}} = \left(\frac{R_{\text{target}} - a}{b}\right)^{-1/c}}_{\text{feedforward}}
$$

$$
\underbrace{I_{\text{pred}}}_{\text{предсказание}} \cdot \underbrace{(1 + u)}_{\text{ПИД-коррекция}} = I_{\text{new}}
$$

**Разделение по времени:**

| Контур | Что делает | Скорость | Данные |
|---|---|---|---|
| EKF | Оценивает $a, b, c$ | Медленно (раз в $N$ ответов) | Вся история |
| Модель | Считает $I_{\text{pred}}$ | Быстро (формула) | — |
| ПИД | Корректирует $I_{\text{pred}}$ | Быстро (каждый ответ) | Последний score |

---

## 8. Система целиком

$$
\boxed{
I_{\text{new}} = \text{clamp}\left[
\left( \frac{R_{\text{target}} - a}{b} \right)^{-1/c}
\cdot
\left( 1 + K_p e + K_i E_{\text{int}} + K_d \Delta e \right),
\ 0.5 I_{\text{old}},\ 2.0 I_{\text{old}}
\right]
}
$$

---

## 9. Дефолтные параметры

Стартовые значения из публичных данных:

$$
\mathbf{x}_0 = \begin{bmatrix} a_0 \\ b_0 \\ c_0 \end{bmatrix}, \quad
\mathbf{P}_0 = \begin{bmatrix} \sigma_a^2 & 0 & 0 \\ 0 & \sigma_b^2 & 0 \\ 0 & 0 & \sigma_c^2 \end{bmatrix}
$$

Широкий prior → EKF почти не доверяет первым данным. По мере накопления $\mathbf{P}$ сжимается, оценки становятся точными.

**Источники данных для калибровки:**

- MaiMemo Open Dataset (220 млн записей);
- FSRS Benchmark Dataset (72 пользователя, 6.2 млн повторений);
- Anki Revlogs (10 000 пользователей);
- Radvansky et al. (2024) — 916 наборов из 256 статей.

---

## 10. Сравнение с SM-2 и FSRS

### 10.1. SM-2

$$
I_{n} = \begin{cases} 1 & n = 1 \\ 6 & n = 2 \\ I_{n-1} \cdot EF & n \geq 3 \end{cases}
$$

$$
EF' = EF + \left( 0.1 - (5 - q)(0.08 + (5 - q) \cdot 0.02) \right)
$$

**Проблемы:** Ease Hell, субъективные оценки 0–5, нет модели памяти.

### 10.2. FSRS

$$
R(t, S) = \left( 1 + \text{FACTOR} \cdot \frac{t}{S} \right)^{\text{DECAY}}
$$

где $\text{FACTOR} = 19/81$, $\text{DECAY} = -0.5$.

**Проблемы:** требует ML и данных, оффлайн-обучение, 17+ параметров, чёрный ящик.

### 10.3. Наш подход

$$
R(t) = a + b \cdot t^{-c}
$$

**Преимущества:** интерпретируемость, онлайн-адаптация, без ML, работает с первого дня.

---

## 11. Обозначения

| Символ | Значение |
|---|---|
| $R(t)$ | вероятность вспомнить через время $t$ |
| $R_{\text{target}}$ | целевой retention (обычно $0.9$) |
| $t$ | время с последнего повторения |
| $a, b, c$ | параметры кривой забывания |
| $I_{\text{pred}}$ | предсказанный интервал |
| $I_{\text{new}}$ | итоговый интервал |
| $e$ | ошибка ПИД |
| $u$ | выход ПИД |
| $K_p, K_i, K_d$ | коэффициенты ПИД |
| $E_{\text{int}}$ | интеграл ошибки |
| $\Delta e$ | сглаженная производная |
| $\gamma$ | коэффициент сглаживания D-члена |
| $m$ | прочность памяти карточки |
| $\tau_{\text{norm}}$ | нормализованное время ответа |
| $\mathbf{x}$ | вектор параметров $(a, b, c)$ |
| $\mathbf{P}$ | ковариация оценки |
| $\mathbf{K}$ | усиление Калмана |
| $\mathbf{H}$ | матрица Якоби |
| $\mathbf{Q}$ | ковариация шума процесса |
| $\sigma_v^2$ | дисперсия шума наблюдения |

---

## 12. План развития

| Итерация | Что делаем |
|---|---|
| **Текущий семестр** | Фиксированные $a, b, c$ из публичных данных. ПИД + feedforward. Сравнение с SM-2 и FSRS. |
| **Следующий семестр** | EKF для онлайн-оценки $a, b, c$. Граф блоков и версионирование. Obsidian-клиент. |
| **Дальше** | Каскадная адаптация. Dual control. Федерация. |

---

## Литература

1. Wixted, J. T., & Ebbesen, E. B. (1991). On the Form of Forgetting. *Psychological Science*, 2(6), 409–415. https://doi.org/10.1111/j.1467-9280.1991.tb00175.x
2. Rubin, D. C., & Wenzel, A. E. (1996). One hundred years of forgetting: A quantitative description of retention. *Psychological Review*, 103(4), 734–760. https://doi.org/10.1037/0033-295X.103.4.734
3. Averell, L., & Heathcote, A. (2011). The form of the forgetting curve and the fate of memories. *Journal of Mathematical Psychology*, 55(1), 25–35. https://doi.org/10.1016/j.jmp.2010.08.001
4. Radvansky, G. A., Parra, D., & Doolen, A. C. (2024). Memory from nonsense syllables to novels: A survey of retention. *Psychonomic Bulletin & Review*, 31(6), 2437–2464. https://doi.org/10.3758/s13423-024-02514-3
5. Murre, J. M. J., & Chessa, A. G. (2011). Power laws from individual differences in learning and forgetting: mathematical analyses. *Psychonomic Bulletin & Review*, 18(3), 592–597. https://doi.org/10.3758/s13423-011-0076-y
6. Fusi, S., Drew, P. J., & Abbott, L. F. (2005). Cascade models of synaptically stored memories. *Neuron*, 45(4), 599–611. https://doi.org/10.1016/j.neuron.2005.02.001
7. Åström, K. J., & Wittenmark, B. (1973). On self tuning regulators. *Automatica*, 9(2), 185–199. https://doi.org/10.1016/0005-1098(73)90074-5
8. Feldbaum, A. A. (1960–1961). Dual Control Theory I–IV. *Automation and Remote Control*, 21–22. (Original: *Avtomatika i Telemekhanika*). Part I: https://www.mathnet.ru/eng/at12624